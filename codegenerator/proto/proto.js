var ejs = require('ejs')
var fs = require('fs')
var helper = require(__dirname + '/helper')

var fileData = process.argv[2]
var pathOutput = process.argv[3]

var fileTemplate = __dirname + '/proto.ejs'

var strData = fs.readFileSync(fileData, 'utf8');
var strTemplate = fs.readFileSync(fileTemplate, 'utf8');

var data = JSON.parse(strData);

var fileOutput = fileData.split('.')[0] + '.proto'

if (pathOutput)
{
    var splitFileOutput   = fileOutput.split('/')
    fileOutput   = pathOutput + '/' + splitFileOutput[splitFileOutput.length - 1]
}

var options = {data:data, helper:helper}
var str = ejs.render(strTemplate, options)

fs.writeFileSync(fileOutput, str, 'utf8');


