use web_sys::{WebSocket, MessageEvent, js_sys};
use wasm_bindgen::prelude::*;
use wasm_bindgen::JsCast;
use std::cell::RefCell;
use crate::log;

// Global WebSocket
thread_local! {
    static WEBSOCKET: RefCell<Option<WebSocket>> = RefCell::new(None);
}

// Initialize WebSocket connection
pub fn init_websocket(url: &str) {
    let ws = WebSocket::new(url).unwrap();

    // Handle incoming messages
    let onmessage = Closure::wrap(Box::new(|e: MessageEvent| {
        if let Ok(txt) = e.data().dyn_into::<js_sys::JsString>() {
            let message = txt.as_string().unwrap();
            log!("WebSocket received: {}", message);

            // Call your message handler
            handle_incoming_message(message);
        }
    }) as Box<dyn FnMut(_)>);

    ws.set_onmessage(Some(onmessage.as_ref().unchecked_ref()));
    onmessage.forget();

    // Connection opened
    let onopen = Closure::wrap(Box::new(|_: MessageEvent| {
        log!("WebSocket connected!");
    }) as Box<dyn FnMut(_)>);

    ws.set_onopen(Some(onopen.as_ref().unchecked_ref()));
    onopen.forget();

    // Store the WebSocket
    WEBSOCKET.with(|ws_ref| {
        *ws_ref.borrow_mut() = Some(ws);
    });
}

// Send message to server
pub fn send_message(msg: &str) {
    WEBSOCKET.with(|ws_ref| {
        if let Some(ws) = ws_ref.borrow().as_ref() {
            log!("WebSocket sending: {}", msg);
            let _ = ws.send_with_str(msg);
        }
    });
}

// Check if connected
pub fn is_connected() -> bool {
    WEBSOCKET.with(|ws_ref| {
        ws_ref.borrow().is_some()
    })
}

// Message handler - customize this for your app
fn handle_incoming_message(message: String) {
    log!("Processing: {}", message);

    // Add your custom logic here
    // For example:
    // - Parse JSON
    // - Update application state
    // - Trigger other functions
    // - etc.
}
