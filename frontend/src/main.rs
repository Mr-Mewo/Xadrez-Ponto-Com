mod app;
use app::App;
mod custom_tags;
mod util;
mod board;
mod tauri_window_buttons;

pub use util::*;

fn main() {
    yew::Renderer::<App>::new().render();
}
