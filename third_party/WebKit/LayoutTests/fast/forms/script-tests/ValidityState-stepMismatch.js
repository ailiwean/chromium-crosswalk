description('Check stepMismatch results for type=date, datetime, datetime-local, month, time, week.');

var input = document.createElement('input');
document.body.appendChild(input);

function stepMismatchFor(value, step, min) {
    input.min = min;
    input.step = step;
    input.value = value;
    return input.validity.stepMismatch;
}

debug('Date type');
input.type = 'date';
debug('Empty values');
shouldBeFalse('stepMismatchFor("", null, null)');
shouldBeFalse('stepMismatchFor("", "2", "1969-12-31")');
debug('Normal step values');
shouldBeTrue('stepMismatchFor("2010-02-10", "2", "2010-02-09")');
shouldBeFalse('stepMismatchFor("2010-02-09", "2", "2010-02-09")');
shouldBeFalse('stepMismatchFor("2010-02-11", "2", "2010-02-09")');
shouldBeTrue('stepMismatchFor("1800-11-11", "3", "1800-11-09")');
shouldBeFalse('stepMismatchFor("1800-11-09", "3", "1800-11-09")');
shouldBeFalse('stepMismatchFor("1800-11-12", "3", "1800-11-09")');
shouldBeTrue('stepMismatchFor("275760-09-13", "3", "275760-09-11")');
shouldBeFalse('stepMismatchFor("275760-09-13", "2", "275760-09-11")');
debug('Implicit step base');
shouldBeTrue('stepMismatchFor("1970-01-02", "2", null)');
shouldBeFalse('stepMismatchFor("1970-01-03", "2", null)');
debug('Fractional step values');
shouldBeFalse('stepMismatchFor("2010-02-10", "0.1", "2010-02-09")');
shouldBeFalse('stepMismatchFor("2010-02-10", "1.1", "2010-02-09")');
shouldBeTrue('stepMismatchFor("2010-02-10", "1.9", "2010-02-09")');
debug('Invalid or no step values');
shouldBeFalse('stepMismatchFor("2010-02-10", null, "2010-02-09")');
shouldBeFalse('stepMismatchFor("2010-02-10", "-1", "2010-02-09")');
shouldBeFalse('stepMismatchFor("2010-02-10", "foo", "2010-02-09")');
debug('Special step value');
shouldBeFalse('stepMismatchFor("2010-02-10", "any", "2010-02-09")');

debug('');
debug('Datetime type');
input.type = 'datetime';
debug('Empty values');
shouldBeFalse('stepMismatchFor("", null, null)');
shouldBeFalse('stepMismatchFor("", "2", "1969-12-31T12:34:56Z")');
debug('Normal step values');
shouldBeFalse('stepMismatchFor("2010-02-09T12:34:55Z", "1", "2010-02-09T12:34:56Z")');
shouldBeTrue('stepMismatchFor("2010-02-09T12:34:55.001Z", "1", "2010-02-09T12:34:56Z")');
shouldBeFalse('stepMismatchFor("2010-02-09T12:34:56.001Z", "0.001", "2010-02-09T12:34:56Z")');
shouldBeTrue('stepMismatchFor("2010-02-09T12:34:55Z", "0.333", "2010-02-09T12:34:56Z")');
shouldBeFalse('stepMismatchFor("2010-02-09T12:34:55.001Z", "0.333", "2010-02-09T12:34:56Z")');
shouldBeFalse('stepMismatchFor("2010-02-09T12:34Z", "86400", "2010-02-08T12:34Z")');
shouldBeTrue('stepMismatchFor("2010-02-09T12:34:56Z", "86400", "2010-02-08T12:34Z")');
shouldBeTrue('stepMismatchFor("275760-09-13T00:00Z", "3", "275760-09-12T23:59:50Z")');
shouldBeFalse('stepMismatchFor("275760-09-13T00:00Z", "2", "275760-09-12T23:59:50Z")');
shouldBeTrue('stepMismatchFor("0001-01-01T00:00Z", "3", "0001-01-11T00:00:02Z")');
shouldBeFalse('stepMismatchFor("0001-01-01T00:00Z", "2", "0001-01-11T00:00:02Z")');
debug('Implicit step base');
shouldBeFalse('stepMismatchFor("1970-01-01T12:34Z", "120", null)');
shouldBeTrue('stepMismatchFor("1970-01-01T12:35Z", "120", null)');
debug('Small step values');
shouldBeFalse('stepMismatchFor("2010-02-10T12:34:56.000Z", "0.0003", "2010-02-10T12:34.55.000Z")');
shouldBeTrue('stepMismatchFor("2010-02-10T12:34:55.001Z", "0.0019", "2010-02-10T12:34.55.000Z")');
debug('Invalid or no step values');
shouldBeFalse('stepMismatchFor("2010-02-10T12:34Z", null, "2010-02-09T12:34Z")');
shouldBeTrue('stepMismatchFor("2010-02-10T12:34:56Z", null, "2010-02-09T12:34Z")');
shouldBeFalse('stepMismatchFor("2010-02-10T12:34Z", "-1", "2010-02-09T12:34Z")');
shouldBeFalse('stepMismatchFor("2010-02-10T12:34Z", "foo", "2010-02-09T12:34Z")');
debug('Special step value');
shouldBeFalse('stepMismatchFor("2010-02-09T12:34Z", "any", "2010-02-09T12:34Z")');

debug('');
debug('Datetime-local type');
input.type = 'datetime-local';
debug('Empty values');
shouldBeFalse('stepMismatchFor("", null, null)');
shouldBeFalse('stepMismatchFor("", "2", "1969-12-31T12:34:56")');
debug('Normal step values');
shouldBeFalse('stepMismatchFor("2010-02-09T12:34:55", "1", "2010-02-09T12:34:56")');
shouldBeTrue('stepMismatchFor("2010-02-09T12:34:55.001", "1", "2010-02-09T12:34:56")');
shouldBeFalse('stepMismatchFor("2010-02-09T12:34:56.001", "0.001", "2010-02-09T12:34:56")');
shouldBeTrue('stepMismatchFor("2010-02-09T12:34:55", "0.333", "2010-02-09T12:34:56")');
shouldBeFalse('stepMismatchFor("2010-02-09T12:34:55.001", "0.333", "2010-02-09T12:34:56")');
shouldBeFalse('stepMismatchFor("2010-02-09T12:34", "86400", "2010-02-08T12:34")');
shouldBeTrue('stepMismatchFor("2010-02-09T12:34:56", "86400", "2010-02-08T12:34")');
shouldBeTrue('stepMismatchFor("275760-09-13T00:00", "3", "275760-09-12T23:59:50")');
shouldBeFalse('stepMismatchFor("275760-09-13T00:00", "2", "275760-09-12T23:59:50")');
shouldBeTrue('stepMismatchFor("0001-01-01T00:00", "3", "0001-01-11T00:00:02")');
shouldBeFalse('stepMismatchFor("0001-01-01T00:00", "2", "0001-01-11T00:00:02")');
debug('Implicit step base');
shouldBeFalse('stepMismatchFor("1970-01-01T12:34", "120", null)');
shouldBeTrue('stepMismatchFor("1970-01-01T12:35", "120", null)');
debug('Small step values');
shouldBeFalse('stepMismatchFor("2010-02-10T12:34:56.000", "0.0003", "2010-02-10T12:34.55.000")');
shouldBeTrue('stepMismatchFor("2010-02-10T12:34:55.001", "0.0019", "2010-02-10T12:34.55.000")');
debug('Invalid or no step values');
shouldBeFalse('stepMismatchFor("2010-02-10T12:34", null, "2010-02-09T12:34")');
shouldBeTrue('stepMismatchFor("2010-02-10T12:34:56", null, "2010-02-09T12:34")');
shouldBeFalse('stepMismatchFor("2010-02-10T12:34", "-1", "2010-02-09T12:34")');
shouldBeFalse('stepMismatchFor("2010-02-10T12:34", "foo", "2010-02-09T12:34")');
debug('Special step value');
shouldBeFalse('stepMismatchFor("2010-02-09T12:34", "any", "2010-02-09T12:34")');

debug('');
debug('Month type');
input.type = 'month';
debug('Empty values');
shouldBeFalse('stepMismatchFor("", null, null)');
shouldBeFalse('stepMismatchFor("", "2", "1969-12")');
debug('Normal step values');
shouldBeTrue('stepMismatchFor("2010-03", "2", "2010-02")');
shouldBeFalse('stepMismatchFor("2010-02", "2", "2010-02")');
shouldBeFalse('stepMismatchFor("2010-04", "2", "2010-02")');
shouldBeTrue('stepMismatchFor("1800-11", "3", "1800-09")');
shouldBeFalse('stepMismatchFor("1800-09", "3", "1800-09")');
shouldBeFalse('stepMismatchFor("1800-12", "3", "1800-09")');
shouldBeTrue('stepMismatchFor("275760-09", "3", "275760-08")');
shouldBeFalse('stepMismatchFor("275760-09", "2", "275760-09")');
debug('Implicit step base');
shouldBeTrue('stepMismatchFor("1970-02", "2", null)');
shouldBeFalse('stepMismatchFor("1970-03", "2", null)');
debug('Fractional step values');
shouldBeFalse('stepMismatchFor("2010-03", "0.1", "2010-02")');
shouldBeFalse('stepMismatchFor("2010-03", "1.1", "2010-02")');
shouldBeTrue('stepMismatchFor("2010-03", "1.9", "2010-02")');
debug('Invalid or no step values');
shouldBeFalse('stepMismatchFor("2010-03", null, "2010-02")');
shouldBeFalse('stepMismatchFor("2010-03", "-1", "2010-02")');
shouldBeFalse('stepMismatchFor("2010-03", "foo", "2010-02")');
debug('Special step value');
shouldBeFalse('stepMismatchFor("2010-03", "any", "2010-02")');

debug('');
debug('Number type');
input.type = 'number';
debug('Empty values');
shouldBe('stepMismatchFor("", null, null)', 'false');
shouldBe('stepMismatchFor("", "1.0", "0.1")', 'false');
debug('Integers');
shouldBe('stepMismatchFor("1", "2", "0")', 'true');
shouldBe('stepMismatchFor("-3", "2", "-4")', 'true');
shouldBe('input.max = "5"; stepMismatchFor("5", "3", "0")', 'true');
shouldBe('input.value', '"5"');
debug('Invalid step values');
input.max = '';
shouldBe('stepMismatchFor("-3", "-2", "-4")', 'false');
shouldBe('stepMismatchFor("-3", null, "-4")', 'false');
shouldBe('stepMismatchFor("-3", undefined, "-4")', 'false');
debug('Huge numbers and small step; uncomparable');
shouldBe('stepMismatchFor("3.40282347e+38", "3", "")', 'false');
shouldBe('stepMismatchFor("3.40282346e+38", "3", "")', 'false');
shouldBe('stepMismatchFor("3.40282345e+38", "3", "")', 'false');
debug('Huge numbers and huge step');
shouldBe('stepMismatchFor("3.20e+38", "0.20e+38", "")', 'false');
shouldBe('stepMismatchFor("3.20e+38", "0.22e+38", "")', 'true');
debug('Fractional numbers');
shouldBe('stepMismatchFor("0.9", "0.1", "")', 'false');
shouldBe('stepMismatchFor("0.9", "0.1000001", "")', 'true');
shouldBe('stepMismatchFor("0.9", "0.1000000000000001", "")', 'false');
shouldBe('stepMismatchFor("1.0", "0.3333333333333333", "")', 'false');

debug('');
debug('Range type');
input.type = 'range';
// The following test inputs are the same as inputs for type=numbe,
// but all expected results should be 'false'.
debug('Empty values');
shouldBe('stepMismatchFor("", null, null)', 'false');
shouldBe('stepMismatchFor("", "1.0", "0.1")', 'false');
debug('Integers');
shouldBe('stepMismatchFor("1", "2", "0")', 'false');
shouldBe('stepMismatchFor("-3", "2", "-4")', 'false');
shouldBe('input.max = "5"; stepMismatchFor("5", "3", "0")', 'false');
shouldBe('input.value', '"3"'); // Different from type=number.
debug('Invalid step values');
input.max = '';
shouldBe('stepMismatchFor("-3", "-2", "-4")', 'false');
shouldBe('stepMismatchFor("-3", null, "-4")', 'false');
shouldBe('stepMismatchFor("-3", undefined, "-4")', 'false');
debug('Huge numbers and small step; uncomparable');
shouldBe('stepMismatchFor("3.40282347e+38", "3", "")', 'false');
shouldBe('stepMismatchFor("3.40282346e+38", "3", "")', 'false');
shouldBe('stepMismatchFor("3.40282345e+38", "3", "")', 'false');
debug('Huge numbers and huge step');
shouldBe('stepMismatchFor("3.20e+38", "0.20e+38", "")', 'false');
shouldBe('stepMismatchFor("3.20e+38", "0.22e+38", "")', 'false');
debug('Fractional numbers');
shouldBe('stepMismatchFor("0.9", "0.1", "")', 'false');
shouldBe('stepMismatchFor("0.9", "0.1000001", "")', 'false');
shouldBe('stepMismatchFor("0.9", "0.1000000000000001", "")', 'false');
shouldBe('stepMismatchFor("1.0", "0.3333333333333333", "")', 'false');

debug('');
debug('Time type');
input.type = 'time';
debug('Empty values');
shouldBeFalse('stepMismatchFor("", null, null)');
shouldBeFalse('stepMismatchFor("", "2", "12:34:56")');
debug('Normal step values');
shouldBeFalse('stepMismatchFor("12:34:55", "1", "12:34:56")');
shouldBeTrue('stepMismatchFor("12:34:55.001", "1", "12:34:56")');
shouldBeFalse('stepMismatchFor("12:34:56.001", "0.001", "12:34:56")');
shouldBeTrue('stepMismatchFor("12:34:55", "0.333", "12:34:56")');
shouldBeFalse('stepMismatchFor("12:34:55.001", "0.333", "12:34:56")');
shouldBeFalse('stepMismatchFor("12:34", "86400", "12:34")');
shouldBeTrue('stepMismatchFor("23:59:59.998", "86399.999", "00:00")');
shouldBeFalse('stepMismatchFor("23:59:59.999", "86399.999", "00:00")');
debug('Implicit step base');
shouldBeFalse('stepMismatchFor("12:34", "120", null)');
shouldBeTrue('stepMismatchFor("12:35", "120", null)');
debug('Small step values');
shouldBeFalse('stepMismatchFor("12:34:55.001", "0.0003", "12:34.55.000")');
shouldBeTrue('stepMismatchFor("12:34:55.001", "0.0019", "12:34.55.000")');
shouldBeFalse('stepMismatchFor("12:34:55.004", "0.0019", "12:34.55.000")');
debug('Invalid or no step values');
shouldBeFalse('stepMismatchFor("12:34", null, "12:34")');
shouldBeTrue('stepMismatchFor("12:34:56", null, "12:34")');
shouldBeFalse('stepMismatchFor("12:34", "-1", "12:34")');
shouldBeFalse('stepMismatchFor("12:34", "foo", "12:34")');
debug('Special step value');
shouldBeFalse('stepMismatchFor("12:35", "any", "12:34")');

debug('');
debug('Week type');
input.type = 'week';
debug('Empty values');
shouldBeFalse('stepMismatchFor("", null, null)');
shouldBeFalse('stepMismatchFor("", "2", "1970-W02")');
debug('Normal step values');
shouldBeTrue('stepMismatchFor("2010-W03", "2", "2010-W02")');
shouldBeFalse('stepMismatchFor("2010-W02", "2", "2010-W02")');
shouldBeFalse('stepMismatchFor("2010-W04", "2", "2010-W02")');
shouldBeTrue('stepMismatchFor("1800-W11", "3", "1800-W09")');
shouldBeFalse('stepMismatchFor("1800-W09", "3", "1800-W09")');
shouldBeFalse('stepMismatchFor("1800-W12", "3", "1800-W09")');
shouldBeTrue('stepMismatchFor("275760-W35", "3", "275760-W33")');
shouldBeFalse('stepMismatchFor("275760-W35", "2", "275760-W33")');
debug('Implicit step base');
shouldBeFalse('stepMismatchFor("1970-W01", "2", null)');
shouldBeTrue('stepMismatchFor("1970-W02", "2", null)');
shouldBeFalse('stepMismatchFor("1970-W03", "2", null)');
shouldBeTrue('stepMismatchFor("1970-W04", "2", null)');
debug('Fractional step values');
shouldBeFalse('stepMismatchFor("2010-W03", "0.1", "2010-W02")');
shouldBeFalse('stepMismatchFor("2010-W03", "1.1", "2010-W02")');
shouldBeTrue('stepMismatchFor("2010-W03", "1.9", "2010-W02")');
debug('Invalid or no step values');
shouldBeFalse('stepMismatchFor("2010-W03", null, "2010-W02")');
shouldBeFalse('stepMismatchFor("2010-W03", "-1", "2010-W02")');
shouldBeFalse('stepMismatchFor("2010-W03", "foo", "2010-W02")');
debug('Special step value');
shouldBeFalse('stepMismatchFor("2010-W03", "any", "2010-W02")');

debug('');
debug('Unsupported types');
shouldBe('input.type = "text"; input.step = "3"; input.min = ""; input.value = "2"; input.validity.stepMismatch', 'false');
shouldBe('input.type = "button"; input.validity.stepMismatch', 'false');
shouldBe('input.type = "checkbox"; input.validity.stepMismatch', 'false');
shouldBe('input.type = "color"; input.validity.stepMismatch', 'false');
shouldBe('input.type = "email"; input.validity.stepMismatch', 'false');
shouldBe('input.type = "hidden"; input.validity.stepMismatch', 'false');
shouldBe('input.type = "image"; input.validity.stepMismatch', 'false');
shouldBe('input.type = "khtml_isindex"; input.validity.stepMismatch', 'false');
shouldBe('input.type = "passwd"; input.validity.stepMismatch', 'false');
shouldBe('input.type = "radio"; input.validity.stepMismatch', 'false');
shouldBe('input.type = "reset"; input.validity.stepMismatch', 'false');
shouldBe('input.type = "search"; input.validity.stepMismatch', 'false');
shouldBe('input.type = "submit"; input.validity.stepMismatch', 'false');
shouldBe('input.type = "tel"; input.validity.stepMismatch', 'false');
shouldBe('input.type = "url"; input.validity.stepMismatch', 'false');
shouldBe('input.type = "file"; input.validity.stepMismatch', 'false');

var successfullyParsed = true;
