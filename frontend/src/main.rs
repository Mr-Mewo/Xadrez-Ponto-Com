mod app;
mod custom_tags;
mod util;
mod game;

fn main() {
    yew::Renderer::< app::App >::new().render();
}
