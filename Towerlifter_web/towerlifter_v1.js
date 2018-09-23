var currentJson;
var dataArray = [];

var autoRefreshInSeconds = 3;
var mobileMultiplier = 3;
var isMouseOver = false;
var isMobile = false;

var currentTowerCommand;
var towerCommandCalled = false;

$(document).ready(function() {
   init();
});

var init = function()
{
    
	if((navigator.userAgent).indexOf("Mobile") > -1)
	{
		isMobile = true;
		autoRefreshInSeconds *= mobileMultiplier;
	}

    window.setInterval(function(){ 
       						if(isMobile || !isMouseOver || !towerCommandCalled) 
    						{ 
                                getContent();
                                updateContent();
    						}}, autoRefreshInSeconds * 1000);        
	
    if(isMobile)
    {
	    window.setInterval(function(){
	    	if(isMouseOver)
	    	{
	    		location.reload();
	    	}
	    }, 1000);
    }

    getContent();
    updateContent();
}

function TowerCommand(command)
{
    towerCommandCalled = true;
    setDivs(command);
    setContent(command);
    currentTowerCommand = command;
    window.setTimeout(function(){ towerCommandCalled = false; }, 3000);
}

var setDivs = function(command)
{
    var button3 = document.getElementById('button3');
    var button1 = document.getElementById('button1');

    if(command == 3)
    {
        button3.className = 'relayButton rounded-corners bon';   
        button1.className = 'relayButton rounded-corners boff';   
    }
    if(command == 1)
    {
        button3.className = 'relayButton rounded-corners boff';   
        button1.className = 'relayButton rounded-corners bon';   
    }
    if(command == 2 || command == 0)
    {
        button3.className = 'relayButton rounded-corners boff';   
        button1.className = 'relayButton rounded-corners boff';   
    }
}

var setStatus = function(status)
{
    var statusButton = document.getElementById('statusButton');
    
    if(status == 1)
        statusButton.innerText = 'UNTEN';
    if(status == 3)
        statusButton.innerText = 'OBEN';
    if(status == 2)
        statusButton.innerText = 'MITTE';
}

function mouseIsOver(){}
function mouseIsOut(){}

function setContent(command) {
	return $.ajax({
		url: configAddress + "/Set/"+command,
		dataType: 'text',
		success: function(string) {

			data = $.parseJSON(string);
			currentJson = data;
			fillData(currentJson.V);
			myResponseHandler();	
		}	
		});
}

function getContent() {

    if(!towerCommandCalled)
    {
        console.log("GetCommandCalled");
	$.ajax({
		dataType: 'text',
   		url: configAddress+'/Get',
   		success: function(string) {
			data = $.parseJSON(string);
			currentJson = data;
			fillData(currentJson.V);
			myResponseHandler();
		}	
	});
    }
}

var myResponseHandler = function(data) {
    currentTowerCommand = dataArray[3];
    updateContent();
}

var fillData = function(myJson)
{
		dataArray = myJson;
}

var updateContent = function()
{
    //currentTowerCommand = dataArray[4];
    setDivs(currentTowerCommand);
    setStatus(dataArray[0]);
}


function mouseIsOver()
{
	isMouseOver = true;
}

function mouseIsOut()
{
	isMouseOver = false;
}