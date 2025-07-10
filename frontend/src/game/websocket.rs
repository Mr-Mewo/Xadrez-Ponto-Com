use web_sys::{WebSocket, MessageEvent, js_sys};
use wasm_bindgen::prelude::*;
use wasm_bindgen::JsCast;
use std::cell::RefCell;
use serde_json::Value;
use crate::game::make_move;
use crate::log;

// The websocket, have benn wanting to call this wet_sock for a while...
// Should I?
thread_local! {
    static WEBSOCKET: RefCell<Option<WebSocket>> = const { RefCell::new(None) };
}

// Puts the dry_sock on the water
pub fn init_websocket(url: &str) {
    let ws = WebSocket::new(url).unwrap();

    // Handle incoming messages
    let onmessage = Closure::wrap(Box::new(|e: MessageEvent| {
        if let Ok(txt) = e.data().dyn_into::<js_sys::JsString>() {
            let message = txt.as_string().unwrap();
            log!("WebSocket received: {}", message);

            let json = serde_json::from_str::<Value>(&message).unwrap_or_else(|_| panic!("Unable to parse {}", message));
            handle_incoming_message(json)
        }
    }) as Box<dyn FnMut(_)>);

    ws.set_onmessage(Some(onmessage.as_ref().unchecked_ref()));
    onmessage.forget();

    // Connected!
    let onopen = Closure::wrap(Box::new(|_: MessageEvent| {
        log!("WebSocket connected!");
    }) as Box<dyn FnMut(_)>);

    ws.set_onopen(Some(onopen.as_ref().unchecked_ref()));
    onopen.forget();

    // makes the wet_sock wetter (updates it)
    WEBSOCKET.with(|ws_ref| {
        *ws_ref.borrow_mut() = Some(ws);
    });
}

// aka: help me I don't know how to play chess!
// Please server-chan help me!
pub fn send_message(msg: &str) {
    WEBSOCKET.with(|ws_ref| {
        if let Some(ws) = ws_ref.borrow().as_ref() {
            log!("WebSocket sending: {}", msg);
            let _ = ws.send_with_str(msg);
        }
    });
}

// Insert a very bad 3AM Kurt Cobain joke here
fn handle_incoming_message(message: Value) {
    log!("Processing...");

    if let Some(msg_type) = message.get("type") {
        if let Some(type_str) = msg_type.as_str() {
            log!("Message type: {}", type_str);

            match type_str {
                "engine-response" => {
                    if let Some(response) = message.get("message").and_then(Value::as_str) {
                        log!("Engine response received");
                        // Extract the best move from the engine output
                        if let Some(best_move) = extract_best_move(response) {
                            log!("Best move: {}", best_move);
                            make_move(&best_move, None).unwrap();
                            // Handle the best move here
                        }
                    }
                },
                "error" => {
                    if let Some(error_msg) = message.get("message").and_then(Value::as_str) {
                        log!("Error: {}", error_msg);
                    }
                },
                _ => {
                    log!("Unknown message type: {}", type_str);
                }
            }
        }
    }
}

// Helper function to extract best move from engine output
fn extract_best_move(engine_output: &str) -> Option<String> {
    for line in engine_output.lines() {
        if line.starts_with("bestmove") {
            let parts: Vec<&str> = line.split_whitespace().collect();
            if parts.len() >= 2 {
                return Some(parts[1].to_string());
            }
        }
    }
    None
}

pub fn is_connected() -> bool {
    WEBSOCKET.with(|ws_ref| {
        ws_ref.borrow().is_some()
    })
}
