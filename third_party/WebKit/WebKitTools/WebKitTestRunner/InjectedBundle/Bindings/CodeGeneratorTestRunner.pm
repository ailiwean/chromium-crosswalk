# Copyright (C) 2010 Apple Inc. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
# BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
# THE POSSIBILITY OF SUCH DAMAGE.

use strict;
use warnings;
use File::Spec;

package CodeGeneratorTestRunner;

sub new
{
    my ($class, $codeGenerator, $outputDir) = @_;

    my $reference = {
        codeGenerator => $codeGenerator,
        outputDir => $outputDir,
    };

    bless($reference, $class);
    return $reference;
}

sub GenerateModule
{
}

sub GenerateInterface
{
    my ($self, $interface, $defines) = @_;

    foreach my $file ($self->_generateHeaderFile($interface), $self->_generateImplementationFile($interface)) {
        open(FILE, ">", File::Spec->catfile($$self{outputDir}, $$file{name})) or die "Failed to open $$file{name} for writing: $!";
        print FILE @{$$file{contents}};
        close(FILE) or die "Failed to close $$file{name} after writing: $!";
    }
}

sub finish
{
}

sub _className
{
    my ($idlType) = @_;

    return "JS" . _implementationClassName($idlType);
}

sub _classRefGetter
{
    my ($self, $idlType) = @_;
    return $$self{codeGenerator}->WK_lcfirst(_implementationClassName($idlType)) . "Class";
}

sub _fileHeaderString
{
    my ($filename) = @_;

    # FIXME: We should pull header out of the IDL file to get the copyright
    # year(s) right.
    return <<EOF;
/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */
EOF
}

sub _generateHeaderFile
{
    my ($self, $interface) = @_;

    my @contents = ();

    my $idlType = $interface->name;
    my $className = _className($idlType);
    my $implementationClassName = _implementationClassName($idlType);
    my $filename = $className . ".h";

    push(@contents, _fileHeaderString($filename));

    my $parentClassName = _parentClassName($interface);

    push(@contents, <<EOF);

#ifndef ${className}_h
#define ${className}_h

#include "${parentClassName}.h"
EOF
    push(@contents, <<EOF);

namespace WTR {

class ${implementationClassName};

class ${className} : public ${parentClassName} {
public:
    static JSClassRef @{[$self->_classRefGetter($idlType)]}();

private:
    static const JSStaticFunction* staticFunctions();
    static const JSStaticValue* staticValues();
EOF

    if (my @functions = @{$interface->functions}) {
        push(@contents, "\n    // Functions\n\n");
        foreach my $function (@functions) {
            push(@contents, "    static JSValueRef @{[$function->signature->name]}(JSContextRef, JSObjectRef, JSObjectRef, size_t, const JSValueRef[], JSValueRef*);\n");
        }
    }

    if (my @attributes = @{$interface->attributes}) {
        push(@contents, "\n    // Attributes\n\n");
        foreach my $attribute (@attributes) {
            push(@contents, "    static JSValueRef @{[$self->_getterName($attribute)]}(JSContextRef, JSObjectRef, JSStringRef, JSValueRef*);\n");
            push(@contents, "    static bool @{[$self->_setterName($attribute)]}(JSContextRef, JSObjectRef, JSStringRef, JSValueRef, JSValueRef*);\n") unless $attribute->type =~ /^readonly/;
        }
    }

    push(@contents, <<EOF);
};
    
${implementationClassName}* to${implementationClassName}(JSContextRef, JSValueRef);

} // namespace WTR

#endif // ${className}_h
EOF

    return { name => $filename, contents => \@contents };
}

sub _generateImplementationFile
{
    my ($self, $interface) = @_;

    my @contentsPrefix = ();
    my %contentsIncludes = ();
    my @contents = ();

    my $idlType = $interface->name;
    my $className = _className($idlType);
    my $implementationClassName = _implementationClassName($idlType);
    my $filename = $className . ".cpp";

    push(@contentsPrefix, _fileHeaderString($filename));

    my $classRefGetter = $self->_classRefGetter($idlType);
    my $parentClassName = _parentClassName($interface);

    $contentsIncludes{"${className}.h"} = 1;
    $contentsIncludes{"${implementationClassName}.h"} = 1;

    push(@contentsPrefix, <<EOF);

EOF

    push(@contents, <<EOF);
#include <JavaScriptCore/JSRetainPtr.h>
#include <wtf/GetPtr.h>

namespace WTR {

${implementationClassName}* to${implementationClassName}(JSContextRef context, JSValueRef value)
{
    if (!context || !value || !${className}::${classRefGetter}() || !JSValueIsObjectOfClass(context, value, ${className}::${classRefGetter}()))
        return 0;
    return static_cast<${implementationClassName}*>(JSWrapper::unwrap(context, value));
}

JSClassRef ${className}::${classRefGetter}()
{
    static JSClassRef jsClass;
    if (!jsClass) {
        JSClassDefinition definition = kJSClassDefinitionEmpty;
        definition.className = "${idlType}";
        definition.parentClass = @{[$self->_parentClassRefGetterExpression($interface)]};
        definition.staticValues = staticValues();
        definition.staticFunctions = staticFunctions();
EOF

    push(@contents, "        definition.initialize = initialize;\n") unless _parentInterface($interface);
    push(@contents, "        definition.finalize = finalize;\n") unless _parentInterface($interface);

    push(@contents, <<EOF);
        jsClass = JSClassCreate(&definition);
    }
    return jsClass;
}

EOF

    push(@contents, $self->_staticFunctionsGetterImplementation($interface), "\n");
    push(@contents, $self->_staticValuesGetterImplementation($interface));

    if (my @functions = @{$interface->functions}) {
        push(@contents, "\n// Functions\n");

        foreach my $function (@functions) {
            push(@contents, <<EOF);

JSValueRef ${className}::@{[$function->signature->name]}(JSContextRef context, JSObjectRef, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    ${implementationClassName}* impl = to${implementationClassName}(context, thisObject);
    if (!impl)
        return JSValueMakeUndefined(context);
EOF
            my @parameters = ();
            my @specifiedParameters = @{$function->parameters};

            push(@contents, "\n") if scalar @specifiedParameters;

            $self->_includeHeaders(\%contentsIncludes, $function->signature->type, $function->signature);

            foreach my $i (0..$#specifiedParameters) {
                my $parameter = $specifiedParameters[$i];

                $self->_includeHeaders(\%contentsIncludes, $idlType, $parameter);

                push(@contents, "    " . $self->_platformTypeVariableDeclaration($parameter, $parameter->name, "arguments[$i]", "argumentCount > $i") . "\n");
                
                push(@parameters, $self->_paramterExpression($parameter));
            }

            my $isVoidReturn = $function->signature->type eq "void";
            my $functionName = "impl->" . $function->signature->name;
            my $functionCall = $functionName . "(" . join(", ", @parameters) . ")";

            push(@contents, "\n") unless scalar @specifiedParameters == 1;
            push(@contents, "    ${functionCall};\n\n") if $isVoidReturn;
            push(@contents, "    return " . $self->_returnExpression($function->signature, $functionCall) . ";\n}\n");
        }
    }

    if (my @attributes = @{$interface->attributes}) {
        push(@contents, "\n// Attributes\n");
        foreach my $attribute (@attributes) {
            $self->_includeHeaders(\%contentsIncludes, $attribute->signature->type, $attribute->signature);

            my $getterName = $self->_getterName($attribute);
            my $getterExpression = "impl->${getterName}()";

            push(@contents, <<EOF);

JSValueRef ${className}::${getterName}(JSContextRef context, JSObjectRef object, JSStringRef, JSValueRef* exception)
{
    ${implementationClassName}* impl = to${implementationClassName}(context, object);
    if (!impl)
        return JSValueMakeUndefined(context);

    return @{[$self->_returnExpression($attribute->signature, $getterExpression)]};
}
EOF

            unless ($attribute->type =~ /^readonly/) {
                push(@contents, <<EOF);

bool ${className}::@{[$self->_setterName($attribute)]}(JSContextRef context, JSObjectRef object, JSStringRef, JSValueRef value, JSValueRef* exception)
{
    ${implementationClassName}* impl = to${implementationClassName}(context, object);
    if (!impl)
        return false;

EOF

                my $platformValue = $self->_platformTypeConstructor($attribute->signature, "value");

                push(@contents, <<EOF);
    impl->@{[$self->_setterName($attribute)]}(${platformValue});

    return true;
}
EOF
            }
        }
    }

    push(@contents, <<EOF);

} // namespace WTR

EOF

    unshift(@contents, map { "#include \"$_\"\n" } sort keys(%contentsIncludes));
    unshift(@contents, @contentsPrefix);

    return { name => $filename, contents => \@contents };
}

sub _getterName
{
    my ($self, $attribute) = @_;

    my $signature = $attribute->signature;
    my $name = $signature->name;

    return $name;
}

sub _includeHeaders
{
    my ($self, $headers, $idlType, $signature) = @_;

    return unless defined $idlType;
    return if $idlType eq "boolean" or $$self{codeGenerator}->IsNonPointerType($idlType);

    $$headers{_className($idlType) . ".h"} = 1;
    $$headers{_implementationClassName($idlType) . ".h"} = 1;
}

sub _implementationClassName
{
    my ($idlType) = @_;

    return $idlType;
}

sub _parentClassName
{
    my ($interface) = @_;

    my $parentInterface = _parentInterface($interface);
    return $parentInterface ? _className($parentInterface) : "JSWrapper";
}

sub _parentClassRefGetterExpression
{
    my ($self, $interface) = @_;

    my $parentInterface = _parentInterface($interface);
    return $parentInterface ? $self->_classRefGetter($parentInterface) . "()" : "0";
}

sub _parentInterface
{
    my ($interface) = @_;
    return $interface->parents->[0];
}

sub _platformType
{
    my ($self, $idlType, $signature) = @_;

    return undef unless defined $idlType;

    return "bool" if $idlType eq "boolean";
    return "JSRetainPtr<JSStringRef>" if $$self{codeGenerator}->IsStringType($idlType);
    return "double" if $$self{codeGenerator}->IsNonPointerType($idlType);
    return _implementationClassName($idlType);
}

sub _platformTypeConstructor
{
    my ($self, $signature, $argumentName) = @_;

    my $idlType = $signature->type;

    return "JSRetainPtr<JSStringRef>(Adopt, JSValueToStringCopy(context, $argumentName, 0))" if $$self{codeGenerator}->IsStringType($idlType);
    return "JSValueToBoolean(context, $argumentName)" if $idlType eq "boolean";
    return "JSValueToNumber(context, $argumentName, 0)" if $$self{codeGenerator}->IsNonPointerType($idlType);
    return "to" . _implementationClassName($idlType) . "(context, $argumentName)";
}

sub _platformTypeVariableDeclaration
{
    my ($self, $signature, $variableName, $argumentName, $condition) = @_;

    my $platformType = $self->_platformType($signature->type, $signature);
    my $constructor = $self->_platformTypeConstructor($signature, $argumentName);

    my %nonPointerTypes = (
        "bool" => 1,
        "double" => 1,
        "JSRetainPtr<JSStringRef>" => 1,
    );

    my $nullValue = "0";
    $nullValue = "$platformType()" if defined $nonPointerTypes{$platformType} && $platformType ne "double";

    $platformType .= "*" unless defined $nonPointerTypes{$platformType};

    return "$platformType $variableName = $condition && $constructor;" if $condition && $platformType eq "bool";
    return "$platformType $variableName = $condition ? $constructor : $nullValue;" if $condition;
    return "$platformType $variableName = $constructor;";
}

sub _returnExpression
{
    my ($self, $signature, $expression) = @_;

    my $convertNullStringAttribute = $signature->extendedAttributes->{"ConvertNullStringTo"};
    my $nullOrEmptyString = "NullStringAsEmptyString";
    $nullOrEmptyString = "NullStringAsNull" if defined $convertNullStringAttribute && $convertNullStringAttribute eq "Null";

    my $returnIDLType = $signature->type;

    return "JSValueMakeUndefined(context)" if $returnIDLType eq "void";
    return "JSValueMakeBoolean(context, ${expression})" if $returnIDLType eq "boolean";
    return "JSValueMakeNumber(context, ${expression})" if $$self{codeGenerator}->IsNonPointerType($returnIDLType);
    return "toJS(context, WTF::getPtr(${expression}))";
}

sub _paramterExpression
{
    my ($self, $parameter) = @_;

    my $idlType = $parameter->type;
    my $name = $parameter->name;

    return "${name}.get()" if $$self{codeGenerator}->IsStringType($idlType);
    return $name;
}

sub _setterName
{
    my ($self, $attribute) = @_;

    my $name = $attribute->signature->name;

    return "set" . $$self{codeGenerator}->WK_ucfirst($name);
}

sub _staticFunctionsGetterImplementation
{
    my ($self, $interface) = @_;

    my $mapFunction = sub {
        my $name = $_->signature->name;
        my @attributes = qw(kJSPropertyAttributeDontDelete kJSPropertyAttributeReadOnly);
        push(@attributes, "kJSPropertyAttributeDontEnum") if $_->signature->extendedAttributes->{"DontEnum"};

        return  "{ \"$name\", $name, " . join(" | ", @attributes) . " }";
    };

    return $self->_staticFunctionsOrValuesGetterImplementation($interface, "function", "{ 0, 0, 0 }", $mapFunction, $interface->functions);
}

sub _staticFunctionsOrValuesGetterImplementation
{
    my ($self, $interface, $functionOrValue, $arrayTerminator, $mapFunction, $functionsOrAttributes) = @_;

    my $className = _className($interface->name);
    my $uppercaseFunctionOrValue = $$self{codeGenerator}->WK_ucfirst($functionOrValue);

    my $result = <<EOF;
const JSStatic${uppercaseFunctionOrValue}* ${className}::static${uppercaseFunctionOrValue}s()
{
EOF

    my @initializers = map(&$mapFunction, @{$functionsOrAttributes});
    return $result . "    return 0;\n}\n" unless @initializers;

    $result .= <<EOF
    static const JSStatic${uppercaseFunctionOrValue} ${functionOrValue}s[] = {
        @{[join(",\n        ", @initializers)]},
        ${arrayTerminator}
    };
    return ${functionOrValue}s;
}
EOF
}

sub _staticValuesGetterImplementation
{
    my ($self, $interface) = @_;

    my $mapFunction = sub {
        return if $_->signature->extendedAttributes->{"NoImplementation"};

        my $attributeName = $_->signature->name;
        my $attributeIsReadonly = $_->type =~ /^readonly/;
        my $getterName = $self->_getterName($_);
        my $setterName = $attributeIsReadonly ? "0" : $self->_setterName($_);
        my @attributes = qw(kJSPropertyAttributeDontDelete);
        push(@attributes, "kJSPropertyAttributeReadOnly") if $attributeIsReadonly;
        push(@attributes, "kJSPropertyAttributeDontEnum") if $_->signature->extendedAttributes->{"DontEnum"};

        return "{ \"$attributeName\", $getterName, $setterName, " . join(" | ", @attributes) . " }";
    };

    return $self->_staticFunctionsOrValuesGetterImplementation($interface, "value", "{ 0, 0, 0, 0 }", $mapFunction, $interface->attributes);
}

1;
