mod app;
use app::App;
mod custom_tags;
mod util;
mod pieces;
mod game;

pub use util::*;

fn main() {
    yew::Renderer::<App>::new().render();
}
