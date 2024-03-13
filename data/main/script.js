var days = ['Sunday','Monday','Tuesday','Wednesday','Thursday','Friday','Saturday'];
var months = ['January','February','March','April','May','June','July','August','September','October','November','December'];
Date.prototype.getMonthName = function() {
    return months[ this.getMonth() ];
};
Date.prototype.getDayName = function() {
    return days[ this.getDay() ];}

/*=============================================================================================================================*/ 
var socket;
let d = new Date();
let deviceSwitch = document.querySelectorAll('.switch')
document.querySelector('.day').innerText=d.getDayName()
setInterval(() => {
let d = new Date()
    document.querySelector('.time').innerText=d.getHours() + ":"+d.getMinutes()
}, 1000);

let sentry_btn = document.getElementById('sentry_mode_btn')
sentry_btn.addEventListener('click',()=>{
    if (sentry_btn.value=='ON') {
        
        sentry_btn.setAttribute('value','OFF')
        sentry_btn.setAttribute('title','Piertotum Locomotor!')
        document.querySelector('.sentry_mode_actv').style.display='none'
        handleSentryEvent('off')

    }
    else{
        sentry_btn.setAttribute('value','ON')
        sentry_btn.setAttribute('title','Finite Incantatem!')
        handleSentryEvent('on')
    }
    sentry_btn.innerText='SENTRY MODE : '+sentry_btn.value
})
function scrollTo(arg){
    document.getElementById(arg).scrollIntoView({behaviour:"smooth"})
    console.log(arg)
}
document.getElementById('Devices').addEventListener('click',()=>{
    scrollTo('devices')
})
document.getElementById('stats').addEventListener('click',()=>{
    scrollTo('info_wrapper')
})
document.getElementById('sentrydiv').addEventListener('click',()=>{
    scrollTo('security')
})
document.getElementById('stats').addEventListener('click',()=>{
    scrollTo('info_wrapper')
})
document.getElementById('log_out').addEventListener('click',()=>{
    window.location.replace(`/`)
})

function websocketInit(){
    socket=new WebSocket('ws://'+window.location.hostname+':81')
    socket.onmessage = function (event) {
        handleEvent(event);
    }    
}

//light controls

const clientEvent = (devId)=>{
    console.log(devId)
    let xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            console.log(this.responseText);
        };
    }
    var url = `toggledevice?devid=${devId}`;
    console.log(url)
    xhr.open("POST", url, true);
    xhr.send();

    console.log("Sendiing data")
}

const onClick = (event)=>{
    
    console.log(event.target.id)
    clientEvent(event.target.id.replace('d',''))

}
deviceSwitch.forEach(element => {
    element.addEventListener('click',onClick); 
});

//updating sensor values on website

function handleEvent(event) {
    let obj = JSON.parse(event.data)
    document.querySelector('.tempi').innerText=obj.Temp;
    document.querySelector('.humidity_data').innerText=obj.Hum;
    document.querySelector('.temp').innerText=`${obj.Temp}°C`;
    document.querySelector('.soil').innerText=`${obj.moist}%`;
    if (obj.door == 1) {
        let door = obj.door
        document.getElementById("door1").setAttribute('d','M 358.206 131.525 L 259.5 155.701 L 260.624 334.772 L 358.206 358.386 L 358.206 131.525 Z')
        document.getElementById('ewhzJgoG8Ss1').style.fill='#067CF8'    
    }
    if (obj.door == 0) {
        let door = obj.door_name
        document.getElementById("door1").setAttribute('d','M358.206278,138.834081h-156.053811v219.551572h156.053809l.000002-219.551572Z')
        document.getElementById('ewhzJgoG8Ss1').style.fill='#067CF8'
    }
}

//sentry event

function handleSentryEvent(){
    let todo = arguments[0];
    let xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            console.log(this.responseText);
            if (todo=='on') {                
                document.querySelector('.sentry_mode_actv').style.display='flex'  //move to sentry mode xhr
            }
        };
    }
    var url = `sentry?do=${todo}`;
    console.log(url)
    xhr.open("POST", url, true);
    xhr.send();

}
window.onload=()=>{
    websocketInit();
}
// document.querySelector('.door').addEventListener('click',function() {
//     // let door = obj.door_name
//     document.getElementById('door2').setAttribute('d','M 358.206 131.525 L 259.5 155.701 L 260.624 334.772 L 358.206 358.386 L 358.206 131.525 Z')
//     setInterval(() => {
//         document.getElementById('door2').setAttribute('d','M358.206278,138.834081h-156.053811v219.551572h156.053809l.000002-219.551572Z')
//     }, 1000);
// })