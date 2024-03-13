const userInfo = {
    USRID: "",
    Pass: ""
};
let loginBtn = document.getElementById("login")
let createBtn = document.getElementById("create")
loginBtn.addEventListener('click', () => {
    logInHandler("login", document.getElementById('login_password'), document.getElementById('login_username'))
})
createBtn.addEventListener('click', () => {
    logInHandler("create", document.getElementById('Newpassword'), document.getElementById('Newusername'))
})
function logInHandler() {
    console.log(arguments[0])
    let passelement = arguments[1]
    let usernameelement = arguments[2]
    let pwd = userInfo.Pass = passelement.value.trim();
    let usrid = userInfo.USRID = usernameelement.value.trim();
    console.log(pwd + usrid)
    passelement.value = '';
    usernameelement.value = '';
    let emptyCondition = usrid == '' || pwd == ''
    let illegalCondition = /'/.test(pwd) || /"/.test(pwd) || /;/.test(pwd) || /:/.test(pwd) || /'/.test(usrid) || /"/.test(usrid) || /;/.test(usrid) || /:/.test(usrid)
    console.log(illegalCondition)
    if ((!emptyCondition && !illegalCondition)) {
        userInfo.USRID = usrid;
        userInfo.Pass = pwd;
        senddata(arguments[0]);
        console.log(userInfo)
    }
    else if (emptyCondition) {
        alert('Password and Username name cannot be empty')
    }
    else if (illegalCondition) {
        alert(`Values cannot include ' " ; :`)
    }
}
function senddata() {
    let todo = arguments[0]
    let xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById('message').innerText=this.responseText
            setTimeout(() => {
                window.location.replace(`/index`)
            }, 1000);
        };
    }
    authData = JSON.stringify(userInfo)
    if (todo == 'create') {
        console.log('request to create')
        var url = `create?id=${userInfo.USRID}&pass=${userInfo.Pass}`;
    }
    if (todo == 'login') {
        console.log('request to login')
        var url = `login?id=${userInfo.USRID}&pass=${userInfo.Pass}`;
    }
    console.log(url)
    xhr.open("POST", url, true);
    xhr.send();
    console.log("Sendiing data")
}