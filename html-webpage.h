
/*
----the "big xml string below once started cannot use // or /* C formats

    CAUTION: inserting classic // comment delimiter in xml def below MAY mess up

    for block comments use 
    <!--
        blah blah blah
        blah blah blah
        blah blah blah
    -->
*/

/*


  OK, ya ready for some fun? HTML + CSS styling + javascript all in and undebuggable environment

  one trick I've learned to how to debug HTML and CSS code.

  get all your HTML code (from html to /html) and past it into this test site
  muck with the HTML and CSS code until it's what you want
  https://www.w3schools.com/html/tryit.asp?filename=tryhtml_intro

  No clue how to debug javascrip other that write, compile, upload, refresh, guess, repeat

  I'm using class designators to set styles and id's for data updating
  for example:
  the CSS class .tabledata defines with the cell will look like
  <td><div class="tabledata" id = "switch"></div></td>

  the XML code will update the data where id = "switch"
  java script then uses getElementById
  document.getElementById("switch").innerHTML="Switch is OFF";


  .. now you can have the class define the look AND the class update the content, but you will then need
  a class for every data field that must be updated, here's what that will look like
  <td><div class="switch"></div></td>

  the XML code will update the data where class = "switch"
  java script then uses getElementsByClassName
  document.getElementsByClassName("switch")[0].style.color=text_color;


  the main general sections of a web page are the following and used here

  <html>
    <style>
    // dump CSS style stuff in here
    </style>
    <body>
      <header>
      // put header code for cute banners here
      </header>
      <main>
      // the buld of your web page contents
      </main>
      <footer>
      // put cute footer (c) 2021 xyz inc type thing
      </footer>
    </body>
    <script>
    // you java code between these tags
    </script>
  </html>


*/

 // note R"KEYWORD( html page code )KEYWORD"; 
 // again I hate strings, so char is it and this method let's us write naturally

const char PAGE_MAIN[] PROGMEM = R"=====(

<!DOCTYPE html>
<html lang="en" class="js-focus-visible">

<title>Web Page Update Demo</title>

  <style>
    table {
      position: relative;
      width:100%;
      border-spacing: 0px;
    }
    tr {
      border: 1px solid white;
      font-family: "Verdana", "Arial", sans-serif;
      font-size: 18px;
    }
    th {
      height: 20px;
      padding: 3px 15px;
      background-color: #343a40;
      color: #FFFFFF !important;
      }
    td {
      height: 20px;
       padding: 3px 15px;
    }
    .tabledata {
      font-size: 24px;
      position: relative;
      padding-left: 5px;
      padding-top: 5px;
      height:   25px;
      border-radius: 5px;
      color: #FFFFFF;
      line-height: 20px;
      transition: all 200ms ease-in-out;
      background-color: #000000;
    }
    .backlightSlider {
      width: 30%;
      height: 55px;
      outline: none;
      height: 25px;
    }
    .bodytext {
      font-family: "Verdana", "Arial", sans-serif;
      font-size: 18px;
      text-align: left;
      font-weight: light;
      border-radius: 5px;
      display:inline;
    }
    .navbar {
      width: 100%;
      height: 50px;
      margin: 0;
      padding: 10px 0px;
      background-color: #FFF;
      color: #000000;
      border-bottom: 5px solid #293578;
    }
    .fixed-top {
      position: fixed;
      top: 0;
      right: 0;
      left: 0;
      z-index: 1030;
    }
    .navtitle {
      float: left;
      height: 50px;
      font-family: "Verdana", "Arial", sans-serif;
      font-size: 50px;
      font-weight: bold;
      line-height: 50px;
      padding-left: 20px;
    }
   .navheading {
     position: fixed;
     left: 60%;
     height: 50px;
     font-family: "Verdana", "Arial", sans-serif;
     font-size: 18px;
     font-weight: bold;
     line-height: 20px;
     padding-right: 20px;
   }
   .navdata {
      justify-content: flex-end;
      position: fixed;
      left: 70%;
      height: 50px;
      font-family: "Verdana", "Arial", sans-serif;
      font-size: 18px;
      font-weight: bold;
      line-height: 20px;
      padding-right: 20px;
   }
    .category {
      font-family: "Verdana", "Arial", sans-serif;
      font-weight: bold;
      font-size: 20px;
      line-height: 50px;
      padding: 20px 10px 0px 10px;
      color: #000000;
    }
    .heading {
      font-family: "Verdana", "Arial", sans-serif;
      font-weight: normal;
      font-size: 20px;
      text-align: left;
    }
  
    .btn {
      background-color: #444444;
      border: none;
      color: white;
      padding: 10px 20px;
      text-align: center;
      text-decoration: none;
      display: inline-block;
      font-size: 16px;
      margin: 4px 2px;
      cursor: pointer;
    }
    .foot {
      font-family: "Verdana", "Arial", sans-serif;
      font-size: 20px;
      position: relative;
      height:   30px;
      text-align: center;   
      color: #AAAAAA;
      line-height: 20px;
    }
    .container {
      max-width: 1800px;
      margin: 0 auto;
    }
    table tr:first-child th:first-child {
      border-top-left-radius: 5px;
    }
    table tr:first-child th:last-child {
      border-top-right-radius: 5px;
    }
    table tr:last-child td:first-child {
      border-bottom-left-radius: 5px;
    }
    table tr:last-child td:last-child {
      border-bottom-right-radius: 5px;
    }
    
  </style>

  <body style="background-color: #efefef" onload="process()">
  
    <header>
      <div class="navbar fixed-top">
          <div class="container">
            <div class="navtitle">Sensor Monitor</div>
            <div class="navdata" id = "date">mm/dd/yyyy</div>
            <div class="navheading">DATE</div><br>
            <div class="navdata" id = "time">00:00:00</div>
            <div class="navheading">TIME</div>
            
          </div>
      </div>
    </header>
  
    <main class="container" style="margin-top:70px">
      <table style="width:50%">
        <colgroup>
         //
         <col span="1" style="background-color:rgb(30,30,30); width: 30%; color:#000000 ;">
         <col span="1" style="background-color:rgb(80,80,80); width: 30%; color:#000000 ;">
         <col span="1" style="background-color:rgb(30,30,30); width: 30%; color:#000000 ;">
         <col span="1" style="background-color:rgb(80,80,80); width: 30%; color:#000000 ;">
        </colgroup>

        <col span="2"style="background-color:rgb(0,0,0); color:#FFFFFF">
        <col span="2"style="background-color:rgb(0,0,0); color:#FFFFFF">
        <col span="2"style="background-color:rgb(0,0,0); color:#FFFFFF">
        <col span="2"style="background-color:rgb(0,0,0); color:#FFFFFF">
        <tr>
        <th colspan="1"><div class="heading">Topic</div></th>
        <th colspan="1"><div class="heading">Value1</div></th>
        <th colspan="1"><div class="heading">Value2</div></th>
        <th colspan="1"><div class="heading">Value3</div></th>
        </tr>

        <tr>
            <td><div class="bodytext">Radar pin 32</div></td>
            <td><div class="tabledata" id = "r1"></div></td>
            <td><div class="tabledata" id = "r2"></div></td>
        </tr>

        <tr>
            <td><div class="bodytext">USB LO/NOW/HI</div></td>
            <td><div class="tabledata" id = "g1"></div></td>
            <td><div class="tabledata" id = "g2"></div></td>
            <td><div class="tabledata" id = "g3"></div></td>
        </tr>

        <tr>
            <td><div class="bodytext">BAT LO/NOW/HI</div></td>
            <td><div class="tabledata" id = "j1"></div></td>
            <td><div class="tabledata" id = "j2"></div></td>
            <td><div class="tabledata" id = "j3"></div></td>
        </tr>


        <tr>
            <td><div class="bodytext">BAT CURRENT</div></td>
            <td><div class="tabledata" id = "c1"></div></td>
            <td><div class="tabledata" id = "c2"></div></td>
            <td><div class="tabledata" id = "c3"></div></td>
        </tr>

        <tr>
            <td><div class="bodytext">DN/HOLD/UP TIME</div></td>
            <td><div class="tabledata" id = "u1"></div></td>
            <td><div class="tabledata" id = "u2"></div></td>
            <td><div class="tabledata" id = "u3"></div></td>
        </tr>


<!--
 // design point 3
 //   top level elements REBOOTS1 and REBOOTS2 are defined in 'design point1'

 //   this is where the the sub elements z1 and z2 are positioned in the web page
 //   in this case the simple *TEXT*  'REBOOTS' is the leftmost item in the row
 //   z1 will appear in the second position (aka REBOOTS1)
 //   z2 will appear next to z1 in the last position (aka REBOOTS2)

 // the bodytext field "REBOOTS" is unrelated to the element names,
 // we just make it similar to make the association between the row and elements be tighter.
-->

        <tr>
            <!-- where description will appear -->
            <td><div class="bodytext"># REBOOTS/BROWNS/DOGS</div></td>

            <!-- where REBOOT1 will appear -->
            <td><div class="tabledata" id = "z1"></div></td>  

            <!-- where REBOOT2 will appear -->
            <td><div class="tabledata" id = "z2"></div></td>  

            <!-- where REBOOT2 will appear -->
            <td><div class="tabledata" id = "z3"></div></td>  
        </tr>


        <tr>
            <td><div class="bodytext">Digital switch</div></td>
            <td><div class="tabledata" id = "switch"></div></td>
        </tr>
      </table>
    </div>

    <button type="button" class = "btn" id = "btn0" onclick="ButtonPress0()">Toggle</button>
</div>

    <br>
            <button type="button" class = "btn" id = "btn1" onclick="ButtonPress1()">Toggle</button>
        </div>
    </br>

    <br>
        <div class="bodytext">Backlight Brightness<span id="brightSlide"></span>)</div>
        <br>
            <input type="range" class="backlightSlider" min="0" max="255" value = "0" width = "0%" oninput="UpdateSlider(this.value)"/>
        </br>
    </br>
  </main>

  <footer div class="foot" id = "temp" >ESP32 Web Page Creation and Data Update Demo</div></footer>
  
  </body>


  <script type = "text/javascript">
  
    // global variable visible to all java functions
    var xmlHttp=createXmlHttpObject();

    // function to create XML object
    function createXmlHttpObject(){
      if(window.XMLHttpRequest){
        xmlHttp=new XMLHttpRequest();
      }
      else{
        xmlHttp=new ActiveXObject("Microsoft.XMLHTTP");
      }
      return xmlHttp;
    }

    // function to handle button press from HTML code above
    // and send a processing string back to server
    // this processing string is use in the .on method
    function ButtonPress0() {
      var xhttp = new XMLHttpRequest(); 
      var message;
      // if you want to handle an immediate reply (like status from the ESP
      // handling of the button press use this code
      // since this button status from the ESP is in the main XML code
      // we don't need this
      // remember that if you want immediate processing feedbac you must send it
      // in the ESP handling function and here
      /*
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          message = this.responseText;
          // update some HTML data
        }
      }
      */
       
      xhttp.open("PUT", "BUTTON_0", false);
      xhttp.send();
    }


    // function to handle button press from HTML code above
    // and send a processing string back to server
    // this processing string is use in the .on method
    function ButtonPress1() {
      var xhttp = new XMLHttpRequest(); 
      /*
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          document.getElementById("button1").innerHTML = this.responseText;
        }
      }
      */
      xhttp.open("PUT", "BUTTON_1", false);
      xhttp.send(); 
    }
    
    function UpdateSlider(value) {
      var xhttp = new XMLHttpRequest();
      // this time i want immediate feedback to the fan speed
      // yea yea yea i realize i'm computing fan speed but the point
      // is how to give immediate feedback
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          // update the web based on reply from  ESP
          document.getElementById("brightSlide").innerHTML=this.responseText;
        }
      }
      // this syntax is really weird the ? is a delimiter
      // first arg UPDATE_SLIDER is use in the .on method
      // server.on("/UPDATE_SLIDER", UpdateSlider);
      // then the second arg VALUE is use in the processing function
      // String t_state = server.arg("VALUE");
      // then + the controls value property
      xhttp.open("PUT", "UPDATE_SLIDER?VALUE="+value, true);
      xhttp.send();
    }

    // function to handle the response from the ESP
    function response(){
      var message;
      var barwidth;
      var currentsensor;
      var xmlResponse;
      var xmldoc;
      var dt = new Date();
      var color = "#e8e8e8";
     
      // get the xml stream
      if(this.responseXML != null){

        xmlResponse=xmlHttp.responseXML;
    
        // get host date and time
        document.getElementById("time").innerHTML = dt.toLocaleTimeString();
        document.getElementById("date").innerHTML = dt.toLocaleDateString();
    
        xmldoc = xmlResponse.getElementsByTagName("RADAR1");
        message = xmldoc[0].firstChild.nodeValue;
        barwidth = message / 40.95;
        document.getElementById("r1").innerHTML=message;
        document.getElementById("r1").style.width=(barwidth+"%");

        // if you want to use global color set above in <style> section
        // other wise uncomment and let the value dictate the color
        //document.getElementById("r1").style.backgroundColor=color;
        //document.getElementById("r1").style.borderRadius="5px";

        //-----------------------------------------------
        
        xmldoc = xmlResponse.getElementsByTagName("RADAR2");
        message = xmldoc[0].firstChild.nodeValue;
        document.getElementById("r2").innerHTML=message;
        document.getElementById("r2").style.width=(barwidth+"%");
        // you can set color dynamically, maybe blue below a value, red above
        document.getElementById("r2").style.backgroundColor=color;
        //document.getElementById("r2").style.borderRadius="5px";

        //-----------------------------------------------

        xmldoc = xmlResponse.getElementsByTagName("USBVOLTAGE1");
        message = xmldoc[0].firstChild.nodeValue;
        document.getElementById("g1").innerHTML=message;
        document.getElementById("g1").style.width=(barwidth+"%");
        // you can set color dynamically, maybe blue below a value, red above
        document.getElementById("g1").style.backgroundColor=color;
        //document.getElementById("g1").style.borderRadius="5px";


        xmldoc = xmlResponse.getElementsByTagName("USBVOLTAGE2");
        message = xmldoc[0].firstChild.nodeValue;
        document.getElementById("g2").innerHTML=message;
        document.getElementById("g2").style.width=(barwidth+"%");
        // you can set color dynamically, maybe blue below a value, red above
        document.getElementById("g2").style.backgroundColor=color;
        //document.getElementById("g2").style.borderRadius="5px";

        xmldoc = xmlResponse.getElementsByTagName("USBVOLTAGE3");
        message = xmldoc[0].firstChild.nodeValue;
        document.getElementById("g3").innerHTML=message;
        document.getElementById("g3").style.width=(barwidth+"%");
        // you can set color dynamically, maybe blue below a value, red above
        document.getElementById("g3").style.backgroundColor=color;
        //document.getElementById("g3").style.borderRadius="5px";

         //-----------------------------------------------

        xmldoc = xmlResponse.getElementsByTagName("BATT_LO");
        message = xmldoc[0].firstChild.nodeValue;
        document.getElementById("j1").innerHTML=message;
        document.getElementById("j1").style.width=(barwidth+"%");
        // you can set color dynamically, maybe blue below a value, red above
        document.getElementById("j1").style.backgroundColor=color;
        //document.getElementById("j1").style.borderRadius="5px";

        xmldoc = xmlResponse.getElementsByTagName("BATT_NO");
        message = xmldoc[0].firstChild.nodeValue;
        document.getElementById("j2").innerHTML=message;
        document.getElementById("j2").style.width=(barwidth+"%");
        // you can set color dynamically, maybe blue below a value, red above
        document.getElementById("j2").style.backgroundColor=color;
        //document.getElementById("j2").style.borderRadius="5px";


        xmldoc = xmlResponse.getElementsByTagName("BATT_HI");
        message = xmldoc[0].firstChild.nodeValue;
        document.getElementById("j3").innerHTML=message;
        document.getElementById("j3").style.width=(barwidth+"%");
        // you can set color dynamically, maybe blue below a value, red above
        document.getElementById("j3").style.backgroundColor=color;
        //document.getElementById("j3").style.borderRadius="5px";


        //-----------------------------------------------

        xmldoc = xmlResponse.getElementsByTagName("BATCURRENT1");
        message = xmldoc[0].firstChild.nodeValue;
        document.getElementById("c1").innerHTML=message;
        document.getElementById("c1").style.width=(barwidth+"%");
        // you can set color dynamically, maybe blue below a value, red above
        document.getElementById("c1").style.backgroundColor=color;
        //document.getElementById("c1").style.borderRadius="5px";

        //-----------------------------------------------

        xmldoc = xmlResponse.getElementsByTagName("BATCURRENT2");
        message = xmldoc[0].firstChild.nodeValue;
        document.getElementById("c2").innerHTML=message;
        document.getElementById("c2").style.width=(barwidth+"%");
        // you can set color dynamically, maybe blue below a value, red above
        document.getElementById("c2").style.backgroundColor=color;
        //document.getElementById("c2").style.borderRadius="5px";

          //-----------------------------------------------
          
          xmldoc = xmlResponse.getElementsByTagName("BATCURRENT3");
          message = xmldoc[0].firstChild.nodeValue;
          document.getElementById("c3").innerHTML=message;
          document.getElementById("c3").style.width=(barwidth+"%");
          // you can set color dynamically, maybe blue below a value, red above
          document.getElementById("c3").style.backgroundColor=color;
          //document.getElementById("c3").style.borderRadius="5px";
          

          //-----------------------------------------------
          
          xmldoc = xmlResponse.getElementsByTagName("UDTIME1");
          message = xmldoc[0].firstChild.nodeValue;
          document.getElementById("u1").innerHTML=message;
          document.getElementById("u1").style.width=(barwidth+"%");
          // you can set color dynamically, maybe blue below a value, red above
          document.getElementById("u1").style.backgroundColor=color;
          //document.getElementById("u1").style.borderRadius="5px";
          
          
          xmldoc = xmlResponse.getElementsByTagName("NOTIME");
          message = xmldoc[0].firstChild.nodeValue;
          document.getElementById("u2").innerHTML=message;
          document.getElementById("u2").style.width=(barwidth+"%");
          // you can set color dynamically, maybe blue below a value, red above
          document.getElementById("u2").style.backgroundColor=color;
          //document.getElementById("u2").style.borderRadius="5px";
          

          xmldoc = xmlResponse.getElementsByTagName("UDTIME2");
          message = xmldoc[0].firstChild.nodeValue;
          document.getElementById("u3").innerHTML=message;
          document.getElementById("u3").style.width=(barwidth+"%");
          // you can set color dynamically, maybe blue below a value, red above
          document.getElementById("u3").style.backgroundColor=color;
          //document.getElementById("u3").style.borderRadius="5px";
          

          //-----------------------------------------------
<!--
 // design point 1
 //  1) define a name for a xml element (REBOOTS1)
 //  2) define sub element for POSITION names (z1)
 //  3) item 1 will be used for sending data across the wire.
 //  4) item 2 will be used internally in this webpage elsewhere for positioning
 //  5) make sure item 2 is carried thru to other weppage locations
-->
      
        xmldoc = xmlResponse.getElementsByTagName("REBOOTS1");
        message = xmldoc[0].firstChild.nodeValue;
        document.getElementById("z1").innerHTML=message;
        document.getElementById("z1").style.width=(barwidth+"%");
        // you can set color dynamically, maybe blue below a value, red above
        document.getElementById("z1").style.backgroundColor=color;
        //document.getElementById("z1").style.borderRadius="5px";

        xmldoc = xmlResponse.getElementsByTagName("REBOOTS2");
        message = xmldoc[0].firstChild.nodeValue;
        document.getElementById("z2").innerHTML=message;
        document.getElementById("z2").style.width=(barwidth+"%");
        // you can set color dynamically, maybe blue below a value, red above
        document.getElementById("z2").style.backgroundColor=color;
        //document.getElementById("z2").style.borderRadius="5px";

        xmldoc = xmlResponse.getElementsByTagName("REBOOTS3");
        message = xmldoc[0].firstChild.nodeValue;
        document.getElementById("z3").innerHTML=message;
        document.getElementById("z3").style.width=(barwidth+"%");
        // you can set color dynamically, maybe blue below a value, red above
        document.getElementById("z3").style.backgroundColor=color;
        //document.getElementById("z3").style.borderRadius="5px";


        //-----------------------------------------------


        xmldoc = xmlResponse.getElementsByTagName("LED");
        message = xmldoc[0].firstChild.nodeValue;
    
        if (message == 0){
          document.getElementById("btn0").innerHTML="LED ON ";
          document.getElementById("btn0").style.borderRadius="10px";

        }
        else{ 
          document.getElementById("btn0").innerHTML="LED OFF";
          document.getElementById("btn0").style.borderRadius="10px";
        }
         
        xmldoc = xmlResponse.getElementsByTagName("SWITCH");
        message = xmldoc[0].firstChild.nodeValue;
        // update the text in the table
        if (message == 0){
          document.getElementById("switch").style.backgroundColor="rgb(200,0,0)";
          document.getElementById("switch").innerHTML="OFF";
           document.getElementById("btn1").innerHTML="RESET NV NOW";
          document.getElementById("btn1").style.borderRadius="10px";
          document.getElementById("switch").style.color="#0000AA"; 
        }
        else {
          document.getElementById("switch").style.backgroundColor="rgb(0,200,0)";
          document.getElementById("switch").innerHTML="ON";
          document.getElementById("btn1").innerHTML="RESET NV OFF";
          document.getElementById("btn1").style.borderRadius="10px";
          document.getElementById("switch").style.color="#00AA00";
        }
      }
     }
  
    // general processing code for the web page to ask for an XML steam
    // this is actually why you need to keep sending data to the page to 
    // force this call with stuff like this
    // server.on("/xml", SendXML);
    // otherwise the page will not request XML from the ESP, and updates will not happen
    function process(){
     
     if(xmlHttp.readyState==0 || xmlHttp.readyState==4) {
        xmlHttp.open("PUT","xml",true);
        xmlHttp.onreadystatechange=response;
        xmlHttp.send(null);
      }       
        // you may have to play with this value, big pages need more porcessing time, and hence
        // a longer timeout
        setTimeout("process()",100);
    }
  
  
  </script>

</html>

)=====";
