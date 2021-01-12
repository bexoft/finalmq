var g_hostname = "";

function setHostname(hostname)
{
    g_hostname = hostname;
    if (g_hostname != '')
    {
        g_hostname += '/';
    }
}

function createRequest()
{
    var xmlhttp;
    if (window.XMLHttpRequest)                                                      
    {// code for IE7+, Firefox, Chrome, Opera, Safari                             
      xmlhttp = new XMLHttpRequest();                                                 
    }                                                                             
    else                                                                            
    {// code for IE6, IE5                                                         
      xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");                               
    }
	xmlhttp.withCredentials = true;
	return xmlhttp;
}

function getCommand(responseText)
{
    return eval('(' + responseText + ')');
}

function callObject(objectname, funcname, inparams, funcresult, context)
{
    var xmlhttp = createRequest();
    xmlhttp.funcresult = funcresult;
    if (funcresult != null)
    {
	    xmlhttp._context = context;
	    xmlhttp.onreadystatechange = function()
	    {
	        if (xmlhttp.readyState == 4 && xmlhttp.status == 200)                             
	        {                                                                           
		    	var command = getCommand(xmlhttp.responseText);
	            var cmd = command[0];
	            var params = command[1];
			    params.callret = cmd.ret;
	            xmlhttp.funcresult(params, xmlhttp._context);
	        }                                                                           
	    }
    }
    xmlhttp.open('POST',g_hostname + 'root.fmq', (funcresult == null) ? false : true);
    var jsonInParams = (inparams == null) ? "" : JSON.stringify(inparams);                               
    xmlhttp.send(objectname + '/' + funcname + jsonInParams + '\r');                                  
	if (funcresult == null)
	{
	    var command = getCommand(xmlhttp.responseText);
	    var cmd = command[0];
	    var params = command[1];
	    params.callret = cmd.ret;
	    return params;
	}
}

