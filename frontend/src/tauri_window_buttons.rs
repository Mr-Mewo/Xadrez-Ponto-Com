use yew::prelude::*;
use wasm_bindgen::JsValue;
use wasm_bindgen_futures::spawn_local;

use crate::{
    invoke,
    callback_without_args as cwa
};

#[function_component(TauriWindowButtons)]
pub fn tauri_window_buttons() -> Html {
    let is_maximized = use_state(|| false);
    
    let minimize = cwa!("minimize");
    
    let maximize = {
        let is_maximized = is_maximized.clone();
        Callback::from(move |_| {
            is_maximized.set(!*is_maximized);
            cwa!("maximize").emit(());
        })
    };
    

    let close = cwa!("close");

    if cfg!(feature = "tauri") {
        html! {
            <div class="tauri-buttons">
                <button onclick={minimize} class="minimize-btn">
                    <img src="assets/img/tauri/minimize.png" />
                </button>

                <button onclick={maximize} class="maximize-btn">
                    <img src={format!("assets/img/tauri/maximize-{}.png", *is_maximized)} />
                </button>

                <button onclick={close} class="close-btn">
                    <img src="assets/img/tauri/close.png" />
                </button>
            </div>
        }
    } else { html! {} }
}

// Possibly removed for good, no real use for it
/*

    let is_fullscreen = use_state(|| false);

    // ------ //

    let fullscreen = {
        let is_fullscreen = is_fullscreen.clone();
        Callback::from(move |_| {
            is_fullscreen.set(!*is_fullscreen);
            cwa!("fullscreen").emit(());

            // log!("{}", *is_fullscreen);
        })
    };

    // ------ //

    <button onclick={fullscreen}>
        <img src={format!("assets/img/tauri/fullscreen-{}.png", *is_fullscreen)} />
    </button>

*/