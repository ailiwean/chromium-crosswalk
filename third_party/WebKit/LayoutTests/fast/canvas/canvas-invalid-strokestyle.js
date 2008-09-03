description("Series of tests to ensure correct behaviour on an invalid strokeStyle()");
var ctx = document.getElementById('canvas').getContext('2d');
ctx.strokeStyle = 'rgb(0, 255, 0)';
ctx.strokeStyle = 'nonsense';
ctx.lineWidth = 200;
ctx.moveTo(0,100);
ctx.lineTo(200,100);
ctx.stroke();
var imageData = ctx.getImageData(0, 0, 200, 200);
var imgdata = imageData.data;
shouldBe("imgdata[4]", "0");
shouldBe("imgdata[5]", "255");
shouldBe("imgdata[6]", "0");

var successfullyParsed = true;
