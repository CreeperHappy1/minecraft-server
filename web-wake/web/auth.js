export async function api(url, options = {}){
    const token = localStorage.getItem("token") ?? "";
    options.headers ??= {};
    options.headers = new Headers(options.headers);
    options.headers.set("Authorization", "Bearer " + token);
    return fetch(url, options);
}

export async function bootstrap(){
    const res = await api("/panel.html");
    document.open();
    const html = await res.text();
    document.write(html);
    document.close();
}