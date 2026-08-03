const token = location.hash.substring(1);
if(token){
    localStorage.setItem("token", token);
    location.replace("/");
}