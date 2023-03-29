var ejs = require('ejs')
var fs = require('fs')
var helper = require(__dirname + '/helper')
var argv = require('minimist')(process.argv.slice(2));

var fileData = argv['input']
var pathOutput = argv['outpath']


var fileTemplate = __dirname + '/proto.ejs'

var strData = fs.readFileSync(fileData, 'utf8');
var strTemplate = fs.readFileSync(fileTemplate, 'utf8');

var data = JSON.parse(strData);

// make the following conversion: ../abc/test.fmq -> ../abc/test
var fileDataSplitPath = fileData.split('/')                                         // fileDataSplitPath = .. abc test.fmq
var fileDataPlain = fileDataSplitPath[fileDataSplitPath.length - 1].split('.')[0]   // fileDataPlain = test
fileDataSplitPath[fileDataSplitPath.length - 1] = fileDataPlain                     // fileDataSplitPath = .. abc test
fileData = fileDataSplitPath.join('/')                                              // fileData = ../abc/test

var fileOutput = fileData + '.proto'

if (pathOutput)
{
    var splitFileOutput   = fileOutput.split('/')
    fileOutput   = pathOutput + '/' + splitFileOutput[splitFileOutput.length - 1]
}

try {
    fs.mkdirSync(pathOutput, { recursive: true })
}
catch (err) {
}

helper.convertTypeId(data)
var options = {data:data, helper:helper}
var str = ejs.render(strTemplate, options)

fs.writeFileSync(fileOutput, str, 'utf8');


