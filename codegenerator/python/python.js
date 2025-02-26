var ejs = require('ejs')
var fs = require('fs')
var helper = require(__dirname + '/helper')
var argv = require('minimist')(process.argv.slice(2));

var fileData = argv['input']
var pathOutput = argv['outpath']

var fileTemplatePY = __dirname + '/python_py.ejs'

var strData = fs.readFileSync(fileData, 'utf8');
var strTemplatePY = fs.readFileSync(fileTemplatePY, 'utf8');

var data = JSON.parse(strData);

var fileOutputPY = fileData + '.py'

if (pathOutput)
{
    var splitFileOutputPY   = fileOutputPY.split('/')

    fileOutputPY   = pathOutput + '/' + splitFileOutputPY[splitFileOutputPY.length - 1]
}

try
{
    fs.mkdirSync(pathOutput, { recursive: true })
}
catch (err)
{
}

var options = {data:data, helper:helper, fileOutputPY:fileOutputPY}
var strPY = ejs.render(strTemplatePY, options)

fs.writeFileSync(fileOutputPY, strPY, 'utf8');


