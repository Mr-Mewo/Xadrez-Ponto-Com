use serde::{Deserialize, Serialize};
use wasm_bindgen::{
    prelude::*,
};
use wasm_bindgen_futures::spawn_local;
use yew::prelude::*;
// My imports
use crate::*;
use crate::custom_tags as ct;

#[function_component(App)]
pub fn app() -> Html {
    let onmousemove = get_mouse_callback();
    
    let px = use_state(|| 4);

    let _px_add = {
        let px = px.clone();
        Callback::from(move |_: i32| px.set(*px + 1))
    };
    let _px_sub = {
        let px = px.clone();
        Callback::from(move |_: i32| px.set(*px - 1))
    };

    html! {
        <main onmousemove={onmousemove}>
            <style>
                { format!(":root{} --px: {} {}", "{", *px, "}") } // ref as to avoid a borrow
            </style>

            // <button onclick={ _px_sub }>{ "-" }</button>
            // <button onclick={ _px_add }>{ "+" }</button>

            <ct::NineSlice src="9sl/tl-green.png" size={*px*12}>

                <div class="header">
                    <div class="title">
                        {"XadrezPontoCom.com"}
                    </div>
                    {
                        if cfg!(feature = "tauri") {
                            let close = callback_without_args!("close");

                            html! {
                                <button onclick={close} style="border: none; background: none; color: white; cursor: pointer;">
                                    {"×"}
                                </button>
                            }
                        } else { html! {} }
                    }
                </div>

                <div class="content">
                    <ct::ChessBoard pixel_size={*px}/>
        
                    <div class="side-panel">

                    </div>
                </div>

                <div class="footer"></div>
            </ct::NineSlice>
        </main>
    }
}