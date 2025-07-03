use yew::prelude::*;
use wasm_bindgen::{JsValue, prelude::wasm_bindgen};
use wasm_bindgen_futures::spawn_local;

#[wasm_bindgen]
extern "C" {
    #[wasm_bindgen(js_namespace = ["window", "__TAURI__", "core"])]
    async fn invoke(cmd: &str, args: JsValue) -> JsValue;
}

macro_rules! tauri_button_callback {
    ($cmd:expr) => {
        Callback::from(|_| {
            spawn_local(async move {
                invoke($cmd, JsValue::NULL).await;
            });
        })
    }
}

#[function_component(TauriWindowButtons)]
pub fn tauri_window_buttons() -> Html {
    let is_maximized = use_state(|| false);
    
    let minimize = tauri_button_callback!("minimize");
    let close = tauri_button_callback!("close");
    let maximize = {
        let is_maximized = is_maximized.clone();
        Callback::from(move |_| {
            is_maximized.set(!*is_maximized);
            tauri_button_callback!("maximize").emit(());
        })
    };

    
    let base_src = "assets/img/header-buttons/".to_string();
    
    html! {
        <>
            <button onclick={minimize} class="minimize-btn">
                <img src={format!("{}minimize.png", base_src)} />
            </button>

            <button onclick={maximize} class="maximize-btn">
                <img src={format!("{}maximize-{}.png", base_src, *is_maximized)} />
            </button>

            <button onclick={close} class="close-btn">
                <img src={format!("{}close.png", base_src)} />
            </button>
        </>
    }
}
