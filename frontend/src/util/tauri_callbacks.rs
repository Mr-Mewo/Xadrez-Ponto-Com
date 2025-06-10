use wasm_bindgen::JsValue;
use wasm_bindgen::prelude::wasm_bindgen;

#[wasm_bindgen]
extern "C" {
    #[wasm_bindgen(js_namespace = ["window", "__TAURI__", "core"])]
    pub async fn invoke(cmd: &str, args: JsValue) -> JsValue;
}

#[macro_export]
macro_rules! callback_without_args {
    ($cmd:expr) => {
        Callback::from(|_| {
            spawn_local(async move {
                invoke($cmd, JsValue::NULL).await;
            });
        })
    }
}