

class FmqSession 
{
    constructor(hostname) 
    {
        this.hostname = '';
        this.sessionId = null;
        if (hostname)
        {
            this.setHostname(hostname);
        }
    }

    setHostname(hostname)
    {
        this.hostname = hostname;
        if (this.hostname != '')
        {
            this.hostname += '/';
        }
    }

    createRequest()
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
        xmlhttp._session = this;
	    return xmlhttp;
    }

    getCommand(responseText)
    {
        return eval('(' + responseText + ')');
    }


    updateSessionId(xmlhttp)
    {
        var sessionId = xmlhttp.getResponseHeader('Set-fmqfcgi-Sessionid');
        if (sessionId)
        {
            this.sessionId = sessionId;
        }
    }

    callObject(objectname, funcname, inparams, funcresult, context)
    {
        var xmlhttp = this.createRequest();
        xmlhttp.funcresult = funcresult;
        if (funcresult != null)
        {
	        xmlhttp._context = context;
	        xmlhttp.onreadystatechange = function()
	        {
	            if (xmlhttp.readyState == 4 && xmlhttp.status == 200)                             
	            {                                                                           
                    xmlhttp._session.updateSessionId(xmlhttp);
		        	var command = xmlhttp._session.getCommand(xmlhttp.responseText);
	                var cmd = command[0];
	                var params = command[1];
			        params.callret = cmd.ret;
	                xmlhttp.funcresult(params, xmlhttp._context);
	            }
	        }
        }
        xmlhttp.open('POST', this.hostname + 'root.fmq', (funcresult == null) ? false : true);
        if (this.sessionId)
        {
            xmlhttp.setRequestHeader('FMQ_SESSIONID', this.sessionId)
        }
        var jsonInParams = (inparams == null) ? "" : JSON.stringify(inparams);                               
        xmlhttp.send(objectname + '/' + funcname + jsonInParams + '\r');                                  
	    if (funcresult == null)
	    {
            this.updateSessionId(xmlhttp);
	        var command = this.getCommand(xmlhttp.responseText);
	        var cmd = command[0];
	        var params = command[1];
	        params.callret = cmd.ret;
	        return params;
	    }
    }


    createSession(funcresult)
    {
        var xmlhttp = this.createRequest();
        xmlhttp.funcresult = funcresult;
        if (funcresult != null)
        {
	        xmlhttp.onreadystatechange = function()
	        {
	            if (xmlhttp.readyState == 4 && xmlhttp.status == 200)                             
	            {                                                                           
                    xmlhttp._session.updateSessionId(xmlhttp);
	                xmlhttp.funcresult();
	            }
	        }
        }
        xmlhttp.open('POST', this.hostname + 'root.fmq', (funcresult == null) ? false : true);
        xmlhttp.setRequestHeader('FMQ_CREATESESSION', 'true');
        xmlhttp.send();                                  
	    if (funcresult == null)
	    {
            this.updateSessionId(xmlhttp);
	    }
    }

    removeSession(funcresult)
    {
        var xmlhttp = this.createRequest();
        xmlhttp.funcresult = funcresult;
        if (funcresult != null)
        {
            xmlhttp.onreadystatechange = function()
            {
                if (xmlhttp.readyState == 4 && xmlhttp.status == 200)                             
                {                                                                           
                    xmlhttp.funcresult();
                }
            }
        }
        xmlhttp.open('POST', this.hostname + 'root.fmq?removesession', (funcresult == null) ? false : true);
        if (this.sessionId)
        {
            xmlhttp.setRequestHeader('FMQ_SESSIONID', this.sessionId)
        }
        xmlhttp.send('');
    }
}






