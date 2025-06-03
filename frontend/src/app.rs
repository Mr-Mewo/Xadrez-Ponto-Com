use wasm_bindgen::{prelude::*, JsCast};
use yew::prelude::*;

use crate::custom_tags as ct;

#[function_component(App)]
pub fn app() -> Html {
    let px = use_state(|| 4);
    
    let px_add = {
        let px = px.clone();
        Callback::from(move |_| px.set(*px + 1))
    };
    let px_sub = {
        let px = px.clone();
        Callback::from(move |_| px.set(*px - 1))
    };
    
    let mut _input = String::new();
    let _letters = String::from("abcdefgh");
    
    html! {
        <main>
            <style>
                { format!(":root{} --px: {} {}", "{", *px, "}") } // ref as to avoid a borrow
            </style>
        
            // <ct::NineSlice src_begin="tiles/blue" size={*px*12}>
            <ct::NineSlice src_begin="blue" size={*px*12}>
                <div class="header">
                    { "Yew Game" }
                </div>
                <div class="content">
                    <div class="board">
                        <button onclick={ px_sub }>{ "-" }</button>
                        <button onclick={ px_add }>{ "+" }</button>
                    </div>
                </div>
                <div class="footer"></div>
            </ct::NineSlice>
        </main>
    }
}
