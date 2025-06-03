mod app;
use app::App;
mod custom_tags;
mod util;

fn main() {
    yew::Renderer::<App>::new().render();
}
