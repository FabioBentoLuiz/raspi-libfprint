var stompClient = null;
var isConnected = false;

function setConnected(connected) {
    $("#enroll").prop("disabled", connected);
    $("#verify").prop("disabled", connected);
    if (connected) {
    	isConnected = true;
    	$("#msgs-list").html("");
    	
    	
    	
        $("#msgs-table").show();
    }
}

function connect(userId) {
    var socket = new SockJS('/libfprint-ws');
    stompClient = Stomp.over(socket);
    stompClient.connect({}, function (frame) {
        setConnected(true);
        console.log('Connected: ' + frame);
        stompClient.subscribe('/queue/user-'+userId, function (message) {
            showMessage(message.body);
            
            if(message.body === "DISCONNECT")
            	disconnect();
        });
    });
}

function disconnect() {
    if (stompClient !== null) {
        stompClient.disconnect();
    }
    setConnected(false);
    console.log("Disconnected");
    isConnected = false;
}

/*function sendName() {
    stompClient.send("/sendMessage", {}, JSON.stringify({'parameter': $("#name").val(), 'id': $("#id").val()}));
}*/

function showMessage(message) {
    $("#msgs-list").append("<tr><td>" + message + "</td></tr>");
}

$(function () {
    $("form").on('submit', function (e) {
        e.preventDefault();
    });
    
    $( "#enroll" ).click(function() { 
    	if(!isConnected){
    		
    		connect($("#userId").val()); 
    		
    		$.get( "/startEnroll?userId="+$("#userId").val() , function(msg) {
      		  console.log( "success startEnroll" );
      		  showMessage(msg);
      		})
      		  .done(function() {
      			  console.log( "done startEnroll" );
      		  })
      		  .fail(function() {
      			  console.log( "error startEnroll" );
      		  })
      		  .always(function() {
      			  console.log( "finished startEnroll" );
      		  });
    	}
    		
    });
    
    $( "#verify" ).click(function() { 
    	if(!isConnected){
    		
    		connect($("#userId").val()); 
    		
    		$.get( "/startVerification?userId="+$("#userId").val() , function(msg) {
      		  console.log( "success startVerification" );
      		  showMessage(msg);
      		})
      		  .done(function() {
      			  console.log( "done startVerification" );
      		  })
      		  .fail(function() {
      			  console.log( "error startVerification" );
      		  })
      		  .always(function() {
      			  console.log( "finished startVerification" );
      		  });
    	}
    });
    
    $( "#identify" ).click(function() { 
    	if(!isConnected){
    		
    		connect(0); 
    		
    		$.get( "/startIdentification" , function(msg) {
      		  console.log( "success startIdentification" );
      		  showMessage(msg);
      		})
      		  .done(function() {
      			  console.log( "done startIdentification" );
      		  })
      		  .fail(function() {
      			  console.log( "error startIdentification" );
      		  })
      		  .always(function() {
      			  console.log( "finished startIdentification" );
      		  });
    	}
    });
});