mod app;
use app::App;
mod custom_tags;
mod util;
mod game;

fn main() {
    yew::Renderer::<App>::new().render();
}
