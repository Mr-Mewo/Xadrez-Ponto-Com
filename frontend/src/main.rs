mod app;
mod custom_tags;
mod util;
mod game;
mod websocket;

fn main() {
    yew::Renderer::< app::App >::new().render();
}
