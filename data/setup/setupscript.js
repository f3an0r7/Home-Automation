const ntwrAuth = {
    SSID: "",
    Pass: ""
};
let submitbtn = document.getElementById('submitbtn')
let openNetworkbtn = document.getElementById('con_to_open')
let restart = document.getElementById('restart')
let formsubmit = () => {
    let pwd = ntwrAuth.Pass = document.getElementById('passform').value.trim();
    let id = ntwrAuth.SSID = document.getElementById('SSIDform').value.trim();
    console.log(pwd + id)
    document.getElementById('passform').value = '';
    document.getElementById('SSIDform').value = '';
    let emptyCondition = pwd == '' || id == ''
    let illegalCondition = /'/.test(pwd) || /"/.test(pwd) || /;/.test(pwd) || /:/.test(pwd) || /'/.test(id) || /"/.test(id) || /;/.test(id) || /:/.test(id)
    console.log(illegalCondition)
    if ((!emptyCondition && !illegalCondition)) {
        ntwrAuth.SSID = id;
        ntwrAuth.Pass = pwd;
        document.querySelector('.loader').style.display = 'block'
        senddata();
    }
    else if (emptyCondition) {
        alert('Password and WiFi name cannot be empty')
    }
    else if (illegalCondition) {
        alert(`values cannot include ' " ; :`)
    }
}
let formsubmit_opennetwrok = () => {
    let pwd = ntwrAuth.Pass = ''
    let id = ntwrAuth.SSID = document.getElementById('SSIDform').value.trim();
    console.log(pwd + id)
    document.getElementById('passform').value = '';
    document.getElementById('SSIDform').value = '';
    let emptyCondition = id == ''
    let illegalCondition = /'/.test(pwd) || /"/.test(pwd) || /;/.test(pwd) || /:/.test(pwd) || /'/.test(id) || /"/.test(id) || /;/.test(id) || /:/.test(id)
    console.log(illegalCondition)
    if ((!emptyCondition && !illegalCondition)) {
        ntwrAuth.SSID = id;
        ntwrAuth.Pass = pwd;
        document.querySelector('.loader').style.display = 'block'
        document.querySelector('.message').innerText = 'Saving Credentials'
        senddata();
    }
    else if (emptyCondition) {
        alert('Password and WiFi name cannot be empty')
    }
    else if (illegalCondition) {
        alert(`values cannot include ' " ; :`)
    }
}
let senddata = () => {
    let xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            document.querySelector('.message').style.display = this.responseText
            document.querySelector('.loader').style.display = 'none'
            console.log(this.responseText);
        };
    }
    authData = JSON.stringify(ntwrAuth)
    var url = `authdata?id=${ntwrAuth.SSID}&pass=${ntwrAuth.Pass}`;
    console.log(url)
    xhr.open("POST", url, true);
    xhr.send();
    console.log("Sendiing data")
}
submitbtn.addEventListener('click', formsubmit)
openNetworkbtn.addEventListener('click', formsubmit_opennetwrok)
restart.addEventListener('click',()=>{
    document.querySelector('.message').innerText = "Requesting restart..."
    document.querySelector('.loader').style.display = 'block'
    let xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            document.querySelector('.message').innerText = this.responseText
            document.querySelector('.loader').style.display = 'none'
            console.log(this.responseText);
        };
    }
    authData = JSON.stringify(ntwrAuth)
    var url = 'restart';
    console.log(url)
    xhr.open("POST", url, true);
    xhr.send();
    console.log("Sendiing data")
})