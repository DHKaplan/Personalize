Pebble.addEventListener('showConfiguration', function(e) {
  // Show config page
  Pebble.openURL('http://www.wticalumni.com/DHK/Personalize2.10.htm');
});


Pebble.addEventListener('webviewclosed',                     
  function(e) 
  { 
  console.log("in webviewclosed");
  var dict = JSON.parse(decodeURIComponent(e.response));
    
  //Send a string to Pebble
  Pebble.sendAppMessage(dict, 
        function(e) 
            { 
            console.log("Send successful."); 
            }, 
        function(e) 
            { 
            console.log("Send failed!"); 
            }); 
  });
