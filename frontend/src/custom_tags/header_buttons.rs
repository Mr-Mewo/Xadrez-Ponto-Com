use std::cmp::{max, min};
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

#[derive(Properties, PartialEq)]
pub struct HeaderButtonsProps {
    pub(crate) px_ref: UseStateHandle<u32>,
}

#[function_component(HeaderButtons)]
pub fn header_buttons(props: &HeaderButtonsProps) -> Html {
    let is_maximized = use_state(|| false);

    let px_add = {
        let px = props.px_ref.clone();
        Callback::from(move |_: MouseEvent| px.set(min(*px + 1, 5)))
    };
    let px_sub = {
        let px = props.px_ref.clone();
        Callback::from(move |_: MouseEvent| px.set(max(*px - 1, 2)))
    };

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
        <div class="header-buttons">
            <button onclick={px_sub} class="minus-btn">
                <img src="assets/img/header-buttons/minus.png"/>
            </button>

            <button onclick={px_add} class="plus-btn">
                <img src="assets/img/header-buttons/plus.png"/>
            </button>

            {
                if cfg!(feature = "tauri") {
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
                } else {
                    html! {}
                }
            }
        </div>
    }
}
