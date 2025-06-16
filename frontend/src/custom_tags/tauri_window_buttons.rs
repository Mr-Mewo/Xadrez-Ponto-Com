use yew::prelude::*;
use wasm_bindgen::JsValue;
use wasm_bindgen_futures::spawn_local;

use crate::{
    invoke, // Required because it's the macro's dependency
    tauri_callback_no_args
};

#[function_component(TauriWindowButtons)]
pub fn tauri_window_buttons() -> Html {
    let is_maximized = use_state(|| false);
    
    let minimize = tauri_callback_no_args!("minimize");
    
    let maximize = {
        let is_maximized = is_maximized.clone();
        Callback::from(move |_| {
            is_maximized.set(!*is_maximized);
            tauri_callback_no_args!("maximize").emit(());
        })
    };
    

    let close = tauri_callback_no_args!("close");
    
    html! {
        <>
            <button onclick={minimize} class="minimize-btn">
                <img src="assets/img/tauri/minimize.png" />
            </button>

            <button onclick={maximize} class="maximize-btn">
                <img src={format!("assets/img/tauri/maximize-{}.png", *is_maximized)} />
            </button>

            <button onclick={close} class="close-btn">
                <img src="assets/img/tauri/close.png" />
            </button>
        </>
    }
}

// Possibly removed for good, no real use for it
/*

    let is_fullscreen = use_state(|| false);

    // ------ //

    let fullscreen = {
        let is_fullscreen = is_fullscreen.clone();
        Callback::from(move |_| {
            is_fullscreen.set(!*is_fullscreen);
            tauri_callback_no_args!("fullscreen").emit(());

            // log!("{}", *is_fullscreen);
        })
    };

    // ------ //

    <button onclick={fullscreen}>
        <img src={format!("assets/img/tauri/fullscreen-{}.png", *is_fullscreen)} />
    </button>

*/