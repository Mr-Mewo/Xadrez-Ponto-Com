use yew::prelude::*;
use wasm_bindgen::JsValue;
use wasm_bindgen_futures::spawn_local;

use crate::{
    invoke,
    callback_without_args as cwa
    // log,
};

#[function_component(TauriWindowButtons)]
pub fn tauri_window_buttons() -> Html {
    let is_maximized = use_state(|| false);
    let is_fullscreen = use_state(|| false);
    
    let minimize = cwa!("minimize");
    
    let maximize = {
        // if *is_fullscreen { return Callback::from(move |_| (
        //
        //     )) };
        
        let is_maximized = is_maximized.clone();
        Callback::from(move |_| {
            is_maximized.set(!*is_maximized);
            cwa!("maximize").emit(());

            // log!("{}", *is_maximized);
        })
    };
    
    let fullscreen = {
        let is_fullscreen = is_fullscreen.clone();
        Callback::from(move |_| {
            is_fullscreen.set(!*is_fullscreen);
            cwa!("fullscreen").emit(());
            
            // log!("{}", *is_fullscreen);
        })
    };
    
    let close = cwa!("close");

    if cfg!(feature = "tauri") {
        html! {
            <div class="tauri-buttons">
                <button onclick={minimize}>
                    <img src="assets/img/tauri/minimize.png" />
                </button>

                <button onclick={maximize}>
                    <img src={format!("assets/img/tauri/maximize-{}.png", *is_maximized)} />
                </button>

                <button onclick={fullscreen}>
                    <img src={format!("assets/img/tauri/fullscreen-{}.png", *is_fullscreen)} />
                </button>

                <button onclick={close}>
                    <img src="assets/img/tauri/close.png" />
                </button>
            </div>
        }
    } else { html! {} }
}