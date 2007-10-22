// 15.4 Array Objects
// (c) 2001 Harri Porten <porten@kde.org>

shouldBe("Array().length", "0");
shouldBe("(new Array()).length", "0");
shouldBe("(new Array(3)).length", "3");
shouldBe("(new Array(11, 22)).length", "2");
shouldBe("(new Array(11, 22))[0]", "11");
shouldBe("Array(11, 22)[1]", "22");
shouldBeUndefined("(new Array(11, 22))[3]");
shouldBe("String(new Array(11, 22))", "'11,22'");
shouldBe("var a = []; a[0] = 33; a[0]", "33");
shouldBe("var a = []; a[0] = 33; a.length", "1");
shouldBe("var a = [11, 22]; a.length = 1; String(a);", "'11'");
shouldBe("var a = [11, 22]; a.length = 1; a.length;", "1");

// range checks
var caught = false;
var ename = "";
try {
  [].length = -1;
} catch (e) {
  // expect Range Error
  caught = true;
  ename = e.name;
}

shouldBeTrue("caught;");
shouldBe("ename", "'RangeError'");

caught = false;
ename = "";
try {
  new Array(Infinity);
} catch (e) {
  // expect Range Error
  caught = true;
  ename = e.name;
}
shouldBeTrue("caught;");
shouldBe("ename", "'RangeError'");

shouldBeUndefined("var a = [11, 22]; a.length = 1; a[1];");
shouldBe("Array().toString()", "''");
shouldBe("Array(3).toString()", "',,'");
shouldBe("Array(11, 22).toString()", "'11,22'");
shouldBe("String(Array(11, 22).concat(33))", "'11,22,33'");
shouldBe("String(Array(2).concat(33, 44))", "',,33,44'");
shouldBe("String(Array(2).concat(Array(2)))", "',,,'");
shouldBe("String(Array(11,22).concat(Array(33,44)))", "'11,22,33,44'");
shouldBe("String(Array(1,2).concat(3,Array(4,5)))", "'1,2,3,4,5'");
shouldBe("var a = new Array(1,2,3); delete a[1]; String(a.concat(4))", "'1,,3,4'");

shouldBe("[1,2,3,4].slice(1, 3).toString()", "'2,3'");
shouldBe("[1,2,3,4].slice(-3, -1).toString()", "'2,3'");
shouldBe("[1,2].slice(-9, 0).length", "0");
shouldBe("[1,2].slice(1).toString()", "'2'");
shouldBe("[1,2].slice().toString()", "'1,2'");

// 2nd set.
shouldBe("(new Array('a')).length", "1");
shouldBe("(new Array('a'))[0]", "'a'");
shouldBeUndefined("(new Array('a'))[1]");

shouldBe("Array('a').length", "1");
shouldBe("Array('a')[0]", "'a'");

shouldBe("String(Array())", "''");
shouldBe("String(Array('a','b'))", "'a,b'");

shouldBe("[].length", "0");
shouldBe("['a'].length", "1");
shouldBe("['a'][0]", "'a'");
shouldBe("['a',,'c'][2]", "'c'");

function forInSum(_a) {
  var s = '';
  for (i in _a)
    s += _a[i];
  return s;
}

shouldBe("forInSum([])", "''");
shouldBe("forInSum(Array())", "''");
shouldBe("forInSum(Array('a'))", "'a'");

var a0 = [];
shouldBe("forInSum(a0)", "''");

var a1 = [ 'a' ];
shouldBe("forInSum(a1)", "'a'");

shouldBe("String([].sort())", "''")
shouldBe("String([3,1,'2'].sort())", "'1,2,3'");
shouldBe("String([,'x','aa'].sort())", "'aa,x,'"); // don't assume 'x'>undefined !

// sort by length
function comp(a, b) {
  var la = String(a).length;
  var lb = String(b).length;
  if (la < lb)
    return -1;
  else if (la > lb)
    return 1;
  else
    return 0;
}
shouldBe("var a = ['aa', 'b', 'cccc', 'ddd']; String(a.sort(comp))", "'b,aa,ddd,cccc'");

// +/-Infinity as function return value
shouldBe("[0, Infinity].sort(function(a, b) { return a - b }).toString()", "'0,Infinity'");

// Array.unshift()
shouldBe("[].unshift('a')", "1");
shouldBe("['c'].unshift('a', 'b')", "3");
shouldBe("var a = []; a.unshift('a'); String(a)", "'a'");
shouldBe("var a = ['c']; a.unshift('a', 'b'); String(a)", "'a,b,c'");

// Array.splice()
shouldBe("String(['a', 'b', 'c'].splice(1, 2, 'x', 'y'))", "'b,c'");

var maxint = Math.pow(2,32)-1;
var arr = new Array();

// 2^32 should not be treated as a valid array index, i.e.
// setting the property on the array should not result in
// the length being modified

arr.length = 40;
arr[maxint] = "test";
shouldBe("arr.length","40");
shouldBe("arr[maxint]","\"test\"");
delete arr[maxint];
shouldBe("arr.length","40");
shouldBe("arr[maxint]","undefined");
arr[maxint-1] = "test2";
shouldBe("arr.length","maxint");
shouldBe("arr[maxint-1]","\"test2\"");

arr = new Array('a','b','c');
arr.__proto__ = { 1: 'x' };
var propnames = new Array();
for (i in arr)
  propnames.push(i);
propnames.sort();
shouldBe("propnames.length","3");
shouldBe("propnames[0]","'0'");
shouldBe("propnames[1]","'1'");
shouldBe("propnames[2]","'2'");

function testToString() {
  // backup
  var backupNumberToString = Number.prototype.toString;
  var backupNumberToLocaleString = Number.prototype.toLocaleString;
  var backupRegExpToString = RegExp.prototype.toString;
  var backupRegExpToLocaleString = RegExp.prototype.toLocaleString;

  // change functions
  Number.prototype.toString = function() { return "toString"; }
  Number.prototype.toLocaleString = function() { return "toLocaleString"; }
  RegExp.prototype.toString = function() { return "toString2"; }
  RegExp.prototype.toLocaleString = function() { return "toLocaleString2"; }

  // the tests
  shouldBe("[1].toString()", "'1'");
  shouldBe("[1].toLocaleString()", "'toLocaleString'");
  Number.prototype.toLocaleString = "invalid";
  shouldBe("[1].toLocaleString()", "'1'");
  shouldBe("[/r/].toString()", "'toString2'");
  shouldBe("[/r/].toLocaleString()", "'toLocaleString2'");
  RegExp.prototype.toLocaleString = "invalid";
  shouldBe("[/r/].toLocaleString()", "'toString2'");

  var caught = false;
  try {
    [{ toString : 0 }].toString();
  } catch (e) {
    caught = true;
  }
  shouldBeTrue("caught");

  // restore
  Number.prototype.toString = backupNumberToString;
  Number.prototype.toLocaleString = backupNumberToLocaleString;
  RegExp.prototype.toString = backupRegExpToString;
  RegExp.prototype.toLocaleString = backupRegExpToLocaleString;
}

successfullyParsed = true
