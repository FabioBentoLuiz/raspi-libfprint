var stompClient = null;

function setConnected(connected) {
    $("#connect").prop("disabled", connected);
    $("#disconnect").prop("disabled", !connected);
    if (connected) {
    	
    	$.get( "/startEnroll?userId="+$("#userId").val() , function(msg) {
    		  console.log( "success startEnroll" );
    		  showGreeting(msg);
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
    	
        $("#msgs-table").show();
    }
    else {
        $("#msgs-table").hide();
    }
    $("#msgs-list").html("");
}

function connect() {
    var socket = new SockJS('/libfprint-ws');
    stompClient = Stomp.over(socket);
    stompClient.connect({}, function (frame) {
        setConnected(true);
        console.log('Connected: ' + frame);
        stompClient.subscribe('/user/queue/from-libfprint', function (greeting) {
            showGreeting(JSON.parse(greeting.body).content);
        });
    });
}

function disconnect() {
    if (stompClient !== null) {
        stompClient.disconnect();
    }
    setConnected(false);
    console.log("Disconnected");
}

function sendName() {
    stompClient.send("/sendMessage", {}, JSON.stringify({'parameter': $("#name").val(), 'id': $("#id").val()}));
}

function showGreeting(message) {
    $("#msgs-list").append("<tr><td>" + message + "</td></tr>");
}

$(function () {
    $("form").on('submit', function (e) {
        e.preventDefault();
    });
    $( "#connect" ).click(function() { connect(); });
    $( "#disconnect" ).click(function() { disconnect(); });
    $( "#send" ).click(function() { sendName(); });
});