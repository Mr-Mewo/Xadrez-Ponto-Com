use wasm_bindgen::prelude::wasm_bindgen;
use web_sys::MouseEvent;
use yew::Callback;
use crate::log;

#[wasm_bindgen]
#[derive(Clone, Copy)]
pub struct Point {
    pub x: i32,
    pub y: i32,
}

#[wasm_bindgen]
impl Point {
    #[wasm_bindgen(constructor)]
    pub fn new(x: i32, y: i32) -> Point {
        Point { x, y }
    }
}

#[wasm_bindgen]
pub fn track_mouse(event: MouseEvent) -> Point {
    let x = event.client_x();
    let y = event.client_y();
    // log!("Mouse position: x={}, y={}", x, y);

    Point::new(x, y)
}

pub fn get_mouse_callback() -> Callback<MouseEvent> {
    Callback::from(|e: MouseEvent| {
        track_mouse(e);
    })
}