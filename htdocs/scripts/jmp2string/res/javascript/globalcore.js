//Provide statistics about fields
function countdatainfields(who, where, ttype) {
    if (ttype == 'val') {
        var frtn = $(who).val();
    } else {
        var frtn = $(who).text();
    }
    var lines = frtn.split("\n");
    var totalchar = lgstrlen(frtn);
    var words = vcountWords(frtn);
    var totalcharswithoutspace = vcountChars(frtn);

    if (totalchar == 0) {
        $(where).text('');
    } else {
        $(where).html(' Chars: ' + totalchar + ' | Chars w\/o space: ' + totalcharswithoutspace + ' | Lines: ' + lines.length + ' | Words: ' + words);
    }
}

function hero_hide() {

    $("#imgB64Alert").hide();
    $("#pophelp").popover('destroy');
    //We destroy any previus popover
    $('#doit').unbind('click');
    //We unbind any functions Do It button has
    $("#shaopt").hide();
    //We make sure SHA options field is hidden
    $("#rptstr").hide();
    //We make sure LoremImpsun options field is hidden
    $("#liopts").hide();
    //We make sure StringReplace options field is hidden
    $("#strrep").hide();
    //Hide bCrypt options
    $("#bCryptDiv").hide();
    //We make sure file field is hidden
    $("#encodeimgbase64f").hide();
    //We make sure repeat_srt field is hidden
    $("#hero").slideUp();
    //hide hero!
    $("#eandd").slideDown();
    $("#btns").show();
    //Show eandd that can be hidden because of base64 image encoder

    //ATTENTION: lgtt must always be on line number 4.
    activefunction = arguments.callee.caller.toString().split(/\r\n|\r|\n/)[3].replace('       lgtt("#action', '').replace('");', '').replace(' ", "', '').replace('lgtt("#action', '').replace(/\r\n|\r|\n/, '');
    //Sets help popover

    //Total Chars in ResultBox - we must set the timeout to prevent it from
    // executing before the encode function
    $("#doit").click(function() {
        setTimeout(function() {
            countdatainfields("#freturn", "#frchartotal", 0);
        }, 50);
    });
}
function lgtt (elementId,name) {
    $(elementId).html($('#'+name).text);
}
//B64E
$("#b64e").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        try {
            var enc = $.base64.encode(eandd);
        } catch (err) {
            var enc = invalid + " - " + err;
            soundalert();
        };
        $("#freturn").text(enc);
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);

});

//B64D
$("#b64d").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        try {
            var dec = $.base64.decode(eandd);
        } catch (err) {
            var dec = invalid + " - " + err;
            soundalert();
        };
        $("#freturn").text(dec);
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
});

//B64EU
$("#b64eu").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        var ec = $.base64.encode(eandd);
        $("#freturn").text(hide64(ec));
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
});

//B64DU
$("#b64du").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        try {
            var dec = show64(eandd);
            var dec2 = $.base64.decode(dec);
        } catch (err) {
            var dec2 = invalid + " - " + err;
            soundalert();
        };
        $("#freturn").text(dec2);
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
});

//B64DIU
$("#b64diu").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        $("#freturn").html('<img src="' + eandd + '" border="0">');
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
});

//ENCODEIMGBASE64
$("#encodeimgbase64").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        try {
            var dec2 = encodeimgbase64();
        } catch (err) {
            var dec2 = invalid + " - " + err;
            soundalert();
        }
        $("#freturn").text(dec2);
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
    $("#eandd").slideUp();
    $("#tbchartotal").text('');
    $("#encodeimgbase64f").show();
    $("#imgB64Alert").fadeIn();
    $("#btns").hide();
});

//DOASCII
$("#doascii").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        $("#freturn").text(DoAsciiHex(eandd, 'A2H'));
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
});

//DOHEX
$("#dohex").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        eandd = eandd.replace(/\s/g, "");
        $("#freturn").text(DoAsciiHex(eandd, 'H2A'));
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
});

//ASCII2BIN
$("#ascii2bin").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        $("#freturn").text(toBinary(eandd));
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
});

//BIN2ASCII
$("#bin2ascii").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        $("#freturn").text(toASCII(eandd));
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
});

//MORSEE
$("#morsee").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        $("#freturn").text(DoMorseEncrypt(eandd));
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
});

//MORSED
$("#morsed").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        $("#freturn").text(DoMorseDecrypt(eandd));
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
});

//CRC32
$("#crc32").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        $("#freturn").text(fcrc32(eandd));
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
});

//bCrypt
$("#bCrypt").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        cryptB();
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
    $("#bCryptDiv").show();
    $("#eandd").hide();
    $("#btns").hide();
});

//MD5H
$("#md5h").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        $("#freturn").text(hex_md5(eandd));
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
});

//MD564
$("#md564").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        $("#freturn").text(b64_md5(eandd));
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
});

//SHA-1
$("#sha1").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        var shaObj = new jsSHA(eandd, "TEXT");

        var opt = $("#shaopt").val();
        var hash = shaObj.getHash("SHA-1", opt);

        $("#freturn").text(hash);
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
    $("#shaopt").show();
});

//SHA-224
$("#sha224").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        var shaObj = new jsSHA(eandd, "TEXT");
        var opt = $("#shaopt").val();
        var hash = shaObj.getHash("SHA-224", opt);
        $("#freturn").text(hash);
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
    $("#shaopt").show();
});

//SHA-256
$("#sha256").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        var shaObj = new jsSHA(eandd, "TEXT");
        var opt = $("#shaopt").val();
        var hash = shaObj.getHash("SHA-256", opt);
        $("#freturn").text(hash);
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
    $("#shaopt").show();
});

//SHA-384
$("#sha384").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        var shaObj = new jsSHA(eandd, "TEXT");
        var opt = $("#shaopt").val();
        var hash = shaObj.getHash("SHA-384", opt);
        $("#freturn").text(hash);
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
    $("#shaopt").show();
});

//SHA-512
$("#sha512").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        var shaObj = new jsSHA(eandd, "TEXT");
        var opt = $("#shaopt").val();
        var hash = shaObj.getHash("SHA-512", opt);
        $("#freturn").text(hash);
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
    $("#shaopt").show();
});

//URLE
$("#urle").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        $("#freturn").text(encodeURIComponent(eandd));
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
});

//IPDE
$("#ipde").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        $("#freturn").text(IPconvert(eandd));
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
});

//URLD
$("#urld").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        $("#freturn").text(decodeURIComponent(eandd));
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
});

//HTMLE
$("#htmle").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        $("#freturn").text(htmlEscape(eandd));
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
});

//HTMLD
$("#htmld").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        $("#freturn").text(htmlUnescape(eandd));
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
});

//XLS2HTML
$("#xls2html").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        $("#freturn").text(xls2html(eandd));
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
});

//EMED
$("#emED").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        try {
            var dec = femED(eandd);
        } catch (err) {
            var dec = invalid;
            soundalert();
        }
        $("#freturn").text(dec);
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
});

//DEMED
$("#dmED").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        try {
            var dec = fdmED(eandd);
        } catch (err) {
            var dec = invalid;
            soundalert();
        }
        $("#freturn").text(dec);
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
});

//ROT13
$("#rot13").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        $("#freturn").text(rot13(eandd));
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
});

//REVE
$("#reve").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        $("#freturn").text(strrev(eandd));
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
});

//C2D
$("#c2d").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        var check = lgcomma2dot(eandd);
        $("#freturn").text(check);
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
});

//STGE
$("#stgs").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        $("#freturn").text(strip_tags(eandd, ''));
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
});

//STRLEN
$("#strlen").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        $("#freturn").text(lgstrlen(eandd));
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
});

//ADDSLASHES
$("#addslashes").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        $("#freturn").text(addslashes(eandd));
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
});

//ADDSLASHES
$("#stripslashes").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        $("#freturn").text(stripslashes(eandd));
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
});

//STRTOLOWER
$("#strtolower").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        $("#freturn").text(strtolower(eandd));
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
});

//STRTOUPPER
$("#strtoupper").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        $("#freturn").text(strtoupper(eandd));
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
});

//UCWORDS
$("#ucwords").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        $("#freturn").text(ucwords(eandd));
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
});

//STR_SHUFFLE
$("#str_shuffle").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        $("#freturn").text(str_shuffle(eandd));
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
});

//CHECKXML
$("#checkxml").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var sMessage = [lgt('validxml'), // 0
            lgt('noroot'), // 1
            lgt('unfinalizedcomment'), // 2
            lgt('unfinalizedcdata'), // 3
            lgt('unexpectedinstruction'), // 4
            lgt('unexpecteddoctype'), // 5
            lgt('text1stag'), // 6
            lgt('textlastag'), // 7
            lgt('unexpectedentity'), // 8
            lgt('morerootnode'), // 9
            lgt('unclodesdtag'), // 10
            lgt('duplicatedattributes')
        ];
        // 11

        var eandd = $("#eandd").val();
        var my_oXmlValidator = new oXmlValidator.Object(eandd);
        my_oXmlValidator.hParams.bFragment = false;
        if (my_oXmlValidator.valid()) {
            $("#freturn").text(lgt('validxml'));
        } else {
            soundinvalid();
            $("#freturn").text(lgt('invalidxml') + ' ' + sMessage[my_oXmlValidator.nCode] + '.');
        }
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
});

//STR_REPEAT
$("#str_repeat").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        var t2r = $("#t2r").val();
        $("#freturn").text(str_repeat(eandd, t2r));
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
    $("#rptstr").show();
});

//STR_REPLACE
$("#str_replace").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        var s2r = $("#strreplace").val();
        var s2f = $("#strflag").val();
        var s2s = $("#strfor").val();
        try {
            var myRegExp = new RegExp(s2r, s2f);
            $("#freturn").text(eandd.replace(myRegExp, s2s));
        } catch (e) {
            $("#freturn").text(lgt('invalidstr'));
            soundinvalid();
        }

        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
    $("#strrep").show();
});

//encodejavascript
$("#escapejavas").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        $("#freturn").text(JAVASstringEncode(eandd));
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
});

//HTML2JS
$("#html2js").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        $("#freturn").text(fhtml2js(eandd));
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
});

//HTML2JS
$("#javascript_escape").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        $("#freturn").text(javascript_escape(eandd));
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
});

//checkUUID
$("#checkUUID").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        if (checkUUID(eandd)) {
            var result = lgt("validUUID");
        } else {
            var result = lgt("invalidUUID");
            soundinvalid();
        }
        $("#freturn").text(result);
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
});

//generateUUID
$("#generateUUID").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    $("#eandd").val("xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx");
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        $("#freturn").text(generateUUID(eandd));
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
});

//TRIM
$("#trim").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        $("#freturn").text(fulltrim(eandd));
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
});

//STRIPSC
$("#stripsc").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        $("#freturn").text(lgstripnonenglish(eandd));
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
});

//LATIN2E
$("#latin2e").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        try {
            var dec = latin2e(eandd);
        } catch (err) {
            var dec = invalid;
            soundalert();
        }
        $("#freturn").text(dec);
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
});

//stripcomments

$("#stripcomments").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var eandd = $("#eandd").val();
        $("#freturn").text(stripcomments(eandd));
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
});

//LOREMIPSUN
$("#loremipsun").click(function() {
    hero_hide();
    //Defines action name
    $('#action').html(this.text);
    //Defines click action
    $("#doit").click(function() {
        var lir = $("#lir").val();
        var lip = $("#lipsel").val();
        var liopt = $("#liopt").val();
        $('#freturn').lorem({
            type: liopt,
            amount: lir,
            ptags: lip
        });
        return false;
    });
    //Show Dyn
    $("#dyn").fadeIn(900);
    $("#eandd").slideUp();
    $("#tbchartotal").text('');
    $("#liopts").show();
    $("#btns").hide();
});

function startstring () {
    $("#navmenutop").fadeIn();
    $("#cfluid").slideDown('slow');
        $("#modalcolors_table").html(rmodalcolors_table());
            $("#modalenti_table").html(rmodalenti_table());

    //Configure Minicolors
    $(".minicolors").minicolors({
        control: 'brightness',
        swatchPosition: 'right',
        textfield: false,
        inline: true,
        change: function(hex, opacity) {
            text = "HEX: ";
            text += hex ? hex : 'transparent';
            if (opacity)
                text += ', ' + opacity;
            text += ' <br> RGBA: ' + $(this).minicolors('rgbaString');

            // Show text in console; disappear after a few seconds
            $('#minicolorsvalue').html(text);

        }
    });

    //Set HeroButtons
    herobuttons();

    //VIEW BUTTON
    $("#view").click(function() {
        var freturn = $("#freturn").html();
        $("#modalhtmlcontent").html('<span class="htmlrender">' + htmlUnescape(freturn) + '</span>');
        $('#modalhtml').modal('show');
    });

    //SAVE BUTTON
    $("#savebtn").click(function() {
        saveFS();
    });

    //ABORT BUTTON
    $("#btnAbort").click(function() {
        aabortRead();
    });

    //Start to load file from Open File Modal
    $("#modalfiles_open").click(function() {
        setTimeout(function() {
            $("#btnAbort").show();
            handleFileSelect();
        }, 390);

    });

    //Total Chars in TextBox
    $("#eandd").keyup(function() {
        countdatainfields("#eandd", "#tbchartotal", 'val');
    });
}