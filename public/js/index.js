var button = document.getElementById('mainButton');
var notice_text = document.getElementById('notice_text');
var name = "";
var slack_user = "";
var position = "";
var socket = io();

//notice_text.hidden();

var openForm = function() {
	button.className = 'active';
};

var checkInput_name = function(input) {
	if (input.value.length > 0) {
		input.className = 'active';
	} else {
		input.className = '';
	}
	name = input.value;
};

var checkInput_usr = function(input) {
	if (input.value.length > 0) {
		input.className = 'active';
	} else {
		input.className = '';
	}
	slack_user = input.value;
};

var checkInput_pos = function(input) {
	if (input.value.length > 0) {
		input.className = 'active';
	} else {
		input.className = '';
	}
	position = input.value;
};
var RequestESP8266 = function() {
	//button.className = '';
	notice_text.style.visibility = 'visible';
	socket.emit('register', {
		'name' : name,
		'slack_user': slack_user,
		'position': position
	});
	name.innerHTML = "tran thanh loc";
	console.log("sent request with: "+name);
};

document.addEventListener("keyup", function(e) {
	if (e.keyCode == 27 || e.keyCode == 13) {
		closeForm();
	}
});

var el = document.getElementById('server-time');

socket.on('complete', function(id) {
	notice_text.innerHTML = 'Complete Register with '+id;
	notice_text.style.color = 'green';
	setTimeout(function () {
        location.reload();
    }, 3000);
});