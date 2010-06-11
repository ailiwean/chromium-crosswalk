description("Tests calling the various uniform[Matrix]* APIs with a null uniform location");

var gl = create3DContext();
var program = loadStandardProgram(gl);

shouldBe("gl.getError()", "gl.NO_ERROR");
shouldBeUndefined("gl.useProgram(program)");
var floatArray = new WebGLFloatArray([1, 2, 3, 4]);
var intArray = new WebGLIntArray([1, 2, 3, 4]);

function callUniformFunction(name) {
    var isArrayVariant = (name.charAt(name.length - 1) == 'v');
    var isMatrix = (name.indexOf("Matrix") != -1);
    var isFloat =
        (name.charAt(name.length - 1) == 'f' ||
         name.charAt(name.length - 2) == 'f');
    var sizeIndex = (isArrayVariant ? name.length - 3 : name.length - 2);
    var size = parseInt(name.substring(sizeIndex, sizeIndex + 1));
    // Initialize argument list with null uniform location
    var args = [ null ];
    if (isArrayVariant) {
        // Call variant which takes values as array
        if (isMatrix) {
            size = size * size;
            args.push(false);
        }
        var array = (isFloat ? new WebGLFloatArray(size) : new WebGLIntArray(size));
        for (var i = 0; i < size; i++) {
            array[i] = i;
        }
        args.push(array);
    } else {
        // Call variant which takes values as parameters
        for (var i = 0; i < size; i++) {
            args.push(i);
        }
    }
    var func = gl[name];
    return func.apply(gl, args);
}

var funcs = [ "uniform1f", "uniform1fv", "uniform1i", "uniform1iv",
              "uniform2f", "uniform2fv", "uniform2i", "uniform2iv",
              "uniform3f", "uniform3fv", "uniform3i", "uniform3iv",
              "uniform4f", "uniform4fv", "uniform4i", "uniform4iv",
              "uniformMatrix2fv", "uniformMatrix3fv", "uniformMatrix4fv" ];
for (var i = 0; i < funcs.length; i++) {
    callString = "callUniformFunction('" + funcs[i] + "')";
    shouldBeUndefined(callString);
    shouldBe("gl.getError()", "gl.NO_ERROR");
}

successfullyParsed = true;
