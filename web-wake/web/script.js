import { api } from "./auth.js";

let wakeButton;
let serverStatus;
let playercount;
let playerList;

async function refreshStatus(){
    const res = await api("/status");
    const status = await res.json();
    serverStatus.textContent = status.status;
    playercount.textContent = status.playercount + "/" + status.maxPlayercount;
    playerList.innerHTML = "";
    for(const player of status.players){
        const item = document.createElement("li");
        item.textContent = player;
        playerList.appendChild(item);
    }
    if(status.status == "OFFLINE" || status.status == "UNKNOWN")
        wakeButton.disabled = false;
    else
        wakeButton.disabled = true;
}

document.addEventListener("DOMContentLoaded", () => {
    wakeButton = document.getElementById("wake");
    serverStatus = document.getElementById("status");
    playercount = document.getElementById("playercount");
    playerList = document.getElementById("players");

    wakeButton.onclick = ()=>{api("/wake", {method: "POST"});};

    refreshStatus();
    setInterval(refreshStatus, 5000);
});