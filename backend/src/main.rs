use tokio::net::{TcpListener, TcpStream};
use tokio::process::Command;
use tokio_tungstenite::{accept_async};
use tokio_tungstenite::tungstenite::Message;
use futures_util::{SinkExt, StreamExt};
#[allow(unused_imports)]
use std::time::Duration;

use serde_json::{Result, Value};

#[tokio::main]
async fn main() {
    let listener = TcpListener::bind("127.0.0.1:7878").await.unwrap();
    println!("WebSocket server listening on 127.0.0.1:7878");

    while let Ok((stream, _)) = listener.accept().await {
        tokio::spawn(handle_connection(stream));
    }
}

async fn handle_connection(stream: TcpStream) {
    let ws_stream = accept_async(stream).await.unwrap();
    let (mut ws_sender, mut ws_receiver) = ws_stream.split();

    while let Some(msg) = ws_receiver.next().await {
        match msg {
            Ok(Message::Text(text)) => {
                println!("Received: {}", text);

                let parsed: Result<Value> = serde_json::from_str(&text);
                let response = match parsed {
                    Ok(json) => {
                        let engine_output = handle_message(json).await;
                        // Use serde_json to properly serialize the response
                        serde_json::json!({
                            "type": "engine-response",
                            "message": engine_output
                        }).to_string()
                    }
                    Err(e) => {
                        serde_json::json!({
                            "type": "error",
                            "message": format!("Error parsing JSON: {}", e)
                        }).to_string()
                    }
                };

                if let Err(e) = ws_sender.send(Message::Text(response.into())).await {
                    println!("Error sending message: {}", e);
                    break;
                }
            }

            Ok(Message::Close(_)) => {
                println!("Connection closed");
                break;
            }

            Err(e) => {
                println!("Error: {}", e);
                break;
            }
            _ => {}
        }
    }
}

async fn handle_message(json: Value) -> String{
    match json.get("type") {
        Some(t) if t == "move" => {
            let fen = json.get("fen").and_then(Value::as_str).unwrap();

            println!("asking for aurora with fen: {}", fen);
            run_aurora_engine(fen).await
        }
        //todo:
        // More message types
        // More stuff to do
        // Damm... Lazy times
        _ => "Unknown message type".to_string(),
    }
}

async fn run_aurora_engine(fen: &str) -> String {
    println!("Starting aurora.exe...");

    let mut cmd = Command::new("engines\\aurora.exe");

    let mut child = match cmd
        .stdin(std::process::Stdio::piped())
        .stdout(std::process::Stdio::piped())
        .stderr(std::process::Stdio::piped())
        .spawn()
    {
        Ok(child) => {
            println!("Aurora.exe started successfully");
            child
        },
        Err(e) => return format!("Failed to start aurora.exe: {}", e),
    };

    let mut stdin = child.stdin.take().unwrap();
    let stdout = child.stdout.take().unwrap();

    // Send initial UCI commands
    use tokio::io::{AsyncWriteExt, AsyncBufReadExt, BufReader};
    let commands = format!("uci\nposition fen {}\ngo depth 8\n", fen);

    if let Err(e) = stdin.write_all(commands.as_bytes()).await {
        return format!("Failed to write to aurora.exe: {}", e);
    }

    // Read output line by line until we find bestmove
    let mut reader = BufReader::new(stdout);
    let mut output_lines = Vec::new();
    let mut line = String::new();

    loop {
        line.clear();
        match reader.read_line(&mut line).await {
            Ok(0) => break, // EOF
            Ok(_) => {
                let trimmed = line.trim().replace("\0", "");
                println!("{}", trimmed);
                output_lines.push(trimmed.to_string());

                // Check if this line contains bestmove
                if trimmed.starts_with("bestmove") {
                    println!("Found bestmove, sending quit command");
                    let _ = stdin.write_all(b"quit\n").await;
                    break;
                }
            }
            Err(e) => {
                println!("Error reading from aurora: {}", e);
                break;
            }
        }
    }

    // Clean up
    drop(stdin);
    let _ = child.wait().await;

    if output_lines.is_empty() {
        "No output from aurora.exe".to_string()
    } else {
        output_lines.join("\n")
    }
}
