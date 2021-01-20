

class FmqEntity
{
    constructor(hostname) 
    {
        this.hostname = '';
        this.sessionId = null;
        if (hostname)
        {
            this._setHostname(hostname);
        }
    }

    _setHostname(hostname)
    {
        this.hostname = hostname;
        if (this.hostname != '')
        {
            this.hostname += '/';
        }
    }

    _createRequest()
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
        xmlhttp._this = this;
	    return xmlhttp;
    }

    _getCommand(responseText)
    {
        return eval('(' + responseText + ')');
    }


    _updateSessionId(xmlhttp)
    {
        var sessionId = xmlhttp.getResponseHeader('Set-fmqfcgi-Sessionid');
        if (sessionId)
        {
            this.sessionId = sessionId;
        }
    }

    requestReply(objectname, funcname, inparams, funcresult, context)
    {
        if (this.sessionId.length == 0)
        {
            return;
        } 
        var xmlhttp = this._createRequest();
        xmlhttp.funcresult = funcresult;
        if (funcresult != null)
        {
	        xmlhttp._context = context;
	        xmlhttp.onreadystatechange = function()
	        {
	            if (xmlhttp.readyState == 4 && xmlhttp.status == 200)                             
	            {                                                                           
                    xmlhttp._this._updateSessionId(xmlhttp);
		        	var command = xmlhttp._this._getCommand(xmlhttp.responseText);
	                var cmd = command[0];
	                var params = command[1];
	                xmlhttp.funcresult(params, xmlhttp._context);
	            }
	        }
        }
        xmlhttp.open('POST', this.hostname + 'root.fmq', (funcresult == null) ? false : true);
        xmlhttp.setRequestHeader('FMQ_SESSIONID', this.sessionId)
        var jsonInParams = (inparams == null) ? "" : JSON.stringify(inparams);                               
        xmlhttp.send('request=' + objectname + '/' + funcname + jsonInParams);
	    if (funcresult == null)
	    {
            this._updateSessionId(xmlhttp);
	        var command = this._getCommand(xmlhttp.responseText);
	        var cmd = command[0];
	        var params = command[1];
	        return params;
	    }
    }

    sendEvent(objectname, funcname, inparams)
    {
        if (this.sessionId.length == 0)
        {
            return;
        } 
        var xmlhttp = this._createRequest();
        xmlhttp.open('POST', this.hostname + 'root.fmq', true);
        xmlhttp.setRequestHeader('FMQ_SESSIONID', this.sessionId)
        var jsonInParams = (inparams == null) ? "" : JSON.stringify(inparams);                               
        xmlhttp.send('request=' + objectname + '/' + funcname + jsonInParams);
    }

    reply(correlationId, funcname, inparams)
    {
        if (this.sessionId.length == 0)
        {
            return;
        } 
        if (correlationId != 0)
        {
            var xmlhttp = this._createRequest();
            xmlhttp.open('POST', this.hostname + 'root.fmq', true);
            xmlhttp.setRequestHeader('FMQ_SESSIONID', this.sessionId)
            var jsonInParams = (inparams == null) ? "" : JSON.stringify(inparams);                               
            xmlhttp.send('reply=' + correlationId + '/' + funcname + jsonInParams);
        }
    }

    replyStatus(correlationId, status)
    {
        if (this.sessionId.length == 0)
        {
            return;
        } 
        if (correlationId != 0)
        {
            var xmlhttp = this._createRequest();
            xmlhttp.open('POST', this.hostname + 'root.fmq', true);
            xmlhttp.setRequestHeader('FMQ_SESSIONID', this.sessionId)
            var jsonInParams = (inparams == null) ? "" : JSON.stringify(inparams);                               
            xmlhttp.send('replystatus=' + correlationId + '/' + status);
        }
    }

    createEntity(funcresult)
    {
        var xmlhttp = this._createRequest();
        xmlhttp.funcresult = funcresult;
        if (funcresult != null)
        {
	        xmlhttp.onreadystatechange = function()
	        {
	            if (xmlhttp.readyState == 4 && xmlhttp.status == 200)                             
	            {                                                                           
                    xmlhttp._this._updateSessionId(xmlhttp);
	                xmlhttp.funcresult();
                    xmlhttp._this._longpoll();
	            }
	        }
        }
        xmlhttp.open('POST', this.hostname + 'root.fmq', (funcresult == null) ? false : true);
        xmlhttp.setRequestHeader('FMQ_CREATESESSION', 'true');
        xmlhttp.send();                                  
	    if (funcresult == null)
	    {
            this._updateSessionId(xmlhttp);
	    }
    }

    removeEntity(funcresult)
    {
        if (this.sessionId.length == 0)
        {
            return;
        } 
        var xmlhttp = this._createRequest();
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
        xmlhttp.setRequestHeader('FMQ_SESSIONID', this.sessionId)
        this.sessionId = '';
        xmlhttp.send('');
    }

    _longpoll()
    {
        if (this.sessionId.length == 0)
        {
            return;
        } 
        var xmlhttp;
        xmlhttp = this._createRequest();
        xmlhttp.onreadystatechange=function()                                           
        {                                                                         
            if (xmlhttp.readyState==4 && xmlhttp.status==200)
            {
                xmlhttp._this._updateSessionId(xmlhttp);
                xmlhttp._this._longpoll();

                var responses = xmlhttp.responseText.split('\n');
                for (var i = 0; i < responses.length; ++i)
                {
                    var response = responses[i];
                    if (response.length > 0)
                    {
                        var command = xmlhttp._this._getCommand(response);                    
	                    var cmd = command[0];
	                    var params = command[1];
                        var methodName = cmd.type.replace(/\./g, '_');  // replace all '.' by '_'
                        var method = xmlhttp._this[methodName];
                        if (method)
                        {
                            method(cmd.src, cmd.corrid, params);
                        }
                        else
                        {
                            xmlhttp._this.replyStatus(cmd.corrid, 'STATUS_REQUEST_NOT_FOUND');
                        }
                    }
                }
            }
        }
        xmlhttp.open("POST", this.hostname + 'root.fmq', true);
        xmlhttp.setRequestHeader('FMQ_SESSIONID', this.sessionId)
        xmlhttp.send('longpoll=10');
    }

}






