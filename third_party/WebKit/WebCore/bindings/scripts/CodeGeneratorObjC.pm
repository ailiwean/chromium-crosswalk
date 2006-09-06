# 
# Copyright (C) 2005 Nikolas Zimmermann <wildfox@kde.org>
# Copyright (C) 2006 Anders Carlsson <andersca@mac.com> 
# Copyright (C) 2006 Samuel Weinig <sam.weinig@gmail.com>
# Copyright (C) 2006 Apple Computer, Inc.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
# 
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
# 
# You should have received a copy of the GNU Library General Public License
# aint with this library; see the file COPYING.LIB.  If not, write to
# the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.
#

package CodeGeneratorObjC;

use File::stat;

my $module = "";
my $outputDir = "";
my %implIncludes = ();
my %headerForwardDeclarations = ();
my %privateHeaderForwardDeclarations = ();
my %headerForwardDeclarationsForProtocols = ();
my %privateHeaderForwardDeclarationsForProtocols = ();
my %publicInterfaces = ();
my $newPublicClass = 0;
my $buildingForTigerOrEarlier = 1 if $ENV{"MACOSX_DEPLOYMENT_TARGET"} and $ENV{"MACOSX_DEPLOYMENT_TARGET"} <= 10.4;
my $buildingForLeopardOrLater = 1 if $ENV{"MACOSX_DEPLOYMENT_TARGET"} and $ENV{"MACOSX_DEPLOYMENT_TARGET"} >= 10.5;

my $exceptionInit = "WebCore::ExceptionCode ec = 0;";
my $exceptionRaiseOnError = "raiseOnDOMError(ec);";

# Default Licence Templates
my $headerLicenceTemplate = << "EOF";
/*
 * Copyright (C) 2004-2006 Apple Computer, Inc.  All rights reserved.
 * Copyright (C) 2006 Samuel Weinig <sam.weinig\@gmail.com>
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
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */
EOF

my $implementationLicenceTemplate = << "EOF";
/*
 * This file is part of the WebKit open source project.
 * This file has been generated by generate-bindings.pl. DO NOT MODIFY!
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
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */
EOF

# Default constructor
sub new
{
    my $object = shift;
    my $reference = { };

    $codeGenerator = shift;
    $outputDir = shift;

    bless($reference, $object);
    return $reference;
}

sub finish
{
    my $object = shift;
}

sub ReadPublicInterfaces
{
    my $class = shift;
    my $superClass = shift;
    my $defines = shift;

    my $found = 0;
    my $actualSuperClass;
    %publicInterfaces = ();

    my $fileName = "WebCore/bindings/objc/PublicDOMInterfaces.h";
    open FILE, "-|", "/usr/bin/gcc", "-E", "-P", "-x", "objective-c", 
        (map { "-D$_" } split(/ /, $defines)), "-DOBJC_CODE_GENERATION", $fileName or die "Could not open $fileName";
    my @documentContent = <FILE>;
    close FILE;

    foreach $line (@documentContent) {
        if ($line =~ /^\s?\@interface $class\s?:\s?(\w+)\s?$/) {
            die "error: Public API change. Superclass for \"$class\" differs ($1 != $superClass)" if $superClass ne $1;
            $found = 1;
            next;
        }

        last if $found and $line =~ /^\s?\@end\s?$/;

        if ($found) {
            # trim whitspace
            $line =~ s/^\s+//;
            $line =~ s/\s+$//;
            $publicInterfaces{$line} = 1;
        }
    }

    # If this class was not found in PublicDOMInterfaces.h then it should be considered as an entirly new public class.
    $newPublicClass = ! $found;
}

# Params: 'domClass' struct
sub GenerateInterface
{
    my $object = shift;
    my $dataNode = shift;
    my $defines = shift;

    $codeGenerator->RemoveExcludedAttributesAndFunctions($dataNode, "ObjC");

    my $name = $dataNode->name;
    my $className = GetClassName($name);
    my $parentClassName = "DOM" . GetParentImplClassName($dataNode);

    ReadPublicInterfaces($className, $parentClassName, $defines);

    # Start actual generation..
    $object->GenerateImplementation($dataNode);
    $object->GenerateHeader($dataNode);

    # Write changes.
    $object->WriteData("DOM" . $name);

    # Check for missing public API
    if (keys %publicInterfaces > 0) {
        my $missing = join( "\n", keys %publicInterfaces );
        die "error: Public API change. There are missing public properties and/or methods from the \"$className\" class.\n$missing\n";
    }
}

# Params: 'idlDocument' struct
sub GenerateModule
{
    my $object = shift;
    my $dataNode = shift;  
    
    $module = $dataNode->module;    
}

sub GetClassName
{
    my $name = $codeGenerator->StripModule(shift);

    # special cases
    if ($name eq "boolean") {
        return "BOOL";
    } elsif ($name eq "unsigned long") {
        return "unsigned";
    } elsif ($name eq "long") {
        return "int";
    } elsif ($name eq "DOMString") {
        return "NSString";
    } elsif ($name eq "URL") {
        return "NSURL";
    } elsif ($name eq "DOMWindow") {
        return "DOMAbstractView";
    } elsif ($name eq "XPathNSResolver") {
        return "id <DOMXPathNSResolver>";
    } elsif ($name eq "unsigned short" 
             or $name eq "float"
             or $name eq "void"
             or $name eq "DOMImplementation") {
        return $name;
    }

    # Default, assume Objective-C type has the same type name as
    # idl type prefixed with "DOM".
    return "DOM" . $name;
}

sub GetClassHeaderName
{
    my $name = shift;
    $name = "DOM" . $name if $name eq "DOMImplementation";
    return $name;
}

sub GetImplClassName
{
    my $name = $codeGenerator->StripModule(shift);

    # special cases
    return "RectImpl" if $name eq "Rect";

    return $name;
}

sub GetParentImplClassName
{
    my $dataNode = shift;

    return "Object" if @{$dataNode->parents} eq 0;

    my $parent = $codeGenerator->StripModule($dataNode->parents(0));

    # special cases
    return "Node" if $parent eq "EventTargetNode";
    return "Object" if $parent eq "HTMLCollection";

    return $parent;
}

sub GetObjCType
{
    my $name = GetClassName(shift);

    if ($codeGenerator->IsPrimitiveType($name)
            or $name eq "BOOL"
            or $name eq "unsigned"
            or $name eq "int"
            or $name eq "id <DOMXPathNSResolver>") {
        return $name;
    }

    # Default, return type as a pointer.
    return "$name *";
}

sub GetObjCTypeMaker
{
    my $type = $codeGenerator->StripModule(shift);

    return "" if $codeGenerator->IsPrimitiveType($type) or $type eq "DOMString" or $type eq "URL";
    return "_RGBColorWithRGB" if $type eq "RGBColor";

    my $typeMaker = "";
    if ($type =~ /^(HTML|CSS)/) {
        $typeMaker = $type; 
    } elsif ($type eq "DOMImplementation") {
        $typeMaker = "DOMImplementation";
    } elsif ($type eq "CDATASection") {
        $typeMaker = "CDATASection";
    } elsif ($type eq "DOMWindow") {
        $typeMaker = "abstractView";
    } elsif ($type eq "XPathResult") {
        $typeMaker = "xpathResult";
    } elsif ($type eq "XPathNSResolver") {
        $typeMaker = "xpathNSResolver";
    } elsif ($type eq "XPathExpression") {
        $typeMaker = "xpathExpression";
    } else {
        $typeMaker = lcfirst($type);
    }

    # put into the form "_fooBarWith" for type FooBar.
    $typeMaker = "_" . $typeMaker . "With";
    return $typeMaker;
}

sub AddForwardDeclarationsForType
{
    my $type = $codeGenerator->StripModule(shift);
    my $public = shift;

    return if $codeGenerator->IsPrimitiveType($type) or $type eq "DOMString";

    if ($type eq "XPathNSResolver") {
        $headerForwardDeclarationsForProtocols{"DOMXPathNSResolver"} = 1 if $public;
        $privateHeaderForwardDeclarationsForProtocols{"DOMXPathNSResolver"} = 1 if !$public and !$headerForwardDeclarationsForProtocols{"DOMXPathNSResolver"};
        return;
    }

    if ($type eq "DOMImplementation") {
        $type = "DOMImplementation";
    } elsif ($type eq "DOMWindow") {
        $type = "DOMAbstractView";
    } else {
        $type = "DOM" . $type;
    }

    $headerForwardDeclarations{$type} = 1 if $public;

    # Private headers include the public header, so only add a forward declaration to the private header
    # if the public header does not already have the same forward declaration.
    $privateHeaderForwardDeclarations{$type} = 1 if !$public and !$headerForwardDeclarations{$type};
}

sub AddIncludesForType
{
    my $type = $codeGenerator->StripModule(shift);

    return if $codeGenerator->IsPrimitiveType($type) or $type eq "URL";

    if ($type eq "DOMString") {
        $implIncludes{"PlatformString.h"} = 1;
        return;
    }

    # Temp DOMCSS.h
    if ($type eq "Rect") {
        $implIncludes{"DOMRect.h"} = 1;
        $implIncludes{"RectImpl.h"} = 1;
        return;
    }

    if ($type eq "RGBColor") {
        $implIncludes{"DOMRGBColor.h"} = 1;
        $implIncludes{"Color.h"} = 1;
        return;
    }

    # Temp DOMEvents.h
    if ($type eq "Event") {
        $implIncludes{"DOMEvents.h"} = 1;
        $implIncludes{"$type.h"} = 1;
        return;
    }

    # Temp DOMViews.h
    if ($type eq "DOMWindow") {
        $implIncludes{"DOMViews.h"} = 1;
        $implIncludes{"$type.h"} = 1;
        return;
    }

    # Temp DOMXPath.h
    if ($type eq "XPathExpression" or $type eq "XPathNSResolver" or $type eq "XPathResult") {
        $implIncludes{"DOMXPath.h"} = 1;
        $implIncludes{"$type.h"} = 1;
        return;
    }

    # FIXME: for some reason it won't compile without both CSSStyleDeclaration.h
    # and CSSMutableStyleDeclaration.h
    if ($type eq "CSSStyleDeclaration") {
        $implIncludes{"CSSMutableStyleDeclaration.h"} = 1;
    }

    # Default, include the same named file (the implementation) and the same name prefixed with "DOM". 
    $implIncludes{"$type.h"} = 1;
    $implIncludes{"DOM$type.h"} = 1;
}

sub GenerateHeader
{
    my $object = shift;
    my $dataNode = shift;

    # Make sure that we don't have more than one parent.
    die "A class can't have more than one parent in ObjC." if @{$dataNode->parents} > 1;

    my $interfaceName = $dataNode->name;
    my $className = GetClassName($interfaceName);
    my $parentClassName = "DOM" . GetParentImplClassName($dataNode);

    my $numConstants = @{$dataNode->constants};
    my $numAttributes = @{$dataNode->attributes};
    my $numFunctions = @{$dataNode->functions};

    # - Add default header template
    @headerContentHeader = split("\r", $headerLicenceTemplate);

    # - INCLUDES -
    my $parentHeaderName = GetClassHeaderName($parentClassName);
    push(@headerContentHeader, "\n#import <WebCore/$parentHeaderName.h>\n\n");

    # - Add constants.
    if ($numConstants > 0) {
        my @headerConstants = ();
        foreach my $constant (@{$dataNode->constants}) {
            my $constantName = $constant->name;
            my $constantValue = $constant->value;
            my $output = "    DOM_" . $constantName . " = " . $constantValue;
            push(@headerConstants, $output);
        }

        my $combinedConstants = join(",\n", @headerConstants);

        # FIXME: the formatting of the enums should line up the equal signs.
        # FIXME: enums are unconditionally placed in the public header.
        push(@headerContent, "enum {\n");
        push(@headerContent, $combinedConstants);
        push(@headerContent, "\n};\n\n");        
    }

    # - Begin @interface 
    push(@headerContent, "\@interface $className : $parentClassName\n");

    my @headerAttributes = ();
    my @privateHeaderAttributes = ();

    # - Add attribute getters/setters.
    if ($numAttributes > 0) {
        foreach my $attribute (@{$dataNode->attributes}) {
            my $attributeName = $attribute->signature->name;

            if ($attributeName eq "id") {
                # Special case attribute id to be idName to avoid Obj-C nameing conflict.
                $attributeName .= "Name";
            } elsif ($attributeName eq "frame") {
                # Special case attribute frame to be frameBorders.
                $attributeName .= "Borders";
            }

            my $attributeType = GetObjCType($attribute->signature->type);
            my $attributeIsReadonly = ($attribute->type =~ /^readonly/);

            my $property = "\@property" . ($attributeIsReadonly ? "(readonly)" : "") . " " . $attributeType . ($attributeType =~ /\*$/ ? "" : " ") . $attributeName . ";";

            my $public = ($publicInterfaces{$property} or $newPublicClass);
            delete $publicInterfaces{$property};

            AddForwardDeclarationsForType($attribute->signature->type, $public);

            if ($buildingForLeopardOrLater) {
                $property .= "\n";
                push(@headerAttributes, $property) if $public;
                push(@privateHeaderAttributes, $property) unless $public;
            } else {
                # - GETTER
                my $getter = "- (" . $attributeType . ")" . $attributeName . ";\n";
                push(@headerAttributes, $getter) if $public;
                push(@privateHeaderAttributes, $getter) unless $public;

                # - SETTER
                if (!$attributeIsReadonly) {
                    my $setter = "- (void)set" . ucfirst($attributeName) . ":(" . $attributeType . ")new" . ucfirst($attributeName) . ";\n";
                    push(@headerAttributes, $setter) if $public;
                    push(@privateHeaderAttributes, $setter) unless $public;
                }
            }
        }

        push(@headerContent, @headerAttributes) if @headerAttributes > 0;
    }

    my @headerFunctions = ();
    my @privateHeaderFunctions = ();
    my @deprecatedHeaderFunctions = ();

    # - Add functions.
    if ($numFunctions > 0) {
        foreach my $function (@{$dataNode->functions}) {
            my $functionName = $function->signature->name;
            my $returnType = GetObjCType($function->signature->type);
            my $needsDeprecatedVersion = (@{$function->parameters} > 1 and $function->signature->extendedAttributes->{"OldStyleObjC"});
            my $numberOfParameters = @{$function->parameters};
            my %typesToForwardDeclare = ($function->signature->type => 1);

            my $parameterIndex = 0;
            my $functionSig = "- ($returnType)$functionName";
            foreach my $param (@{$function->parameters}) {
                my $paramName = $param->name;
                my $paramType = GetObjCType($param->type);

                $typesToForwardDeclare{$param->type} = 1;

                if ($parameterIndex >= 1) {
                    my $paramPrefix = $param->extendedAttributes->{"ObjCPrefix"};
                    $paramPrefix = $paramName unless defined($paramPrefix);
                    $functionSig .= " $paramPrefix";
                }

                $functionSig .= ":($paramType)$paramName";

                $parameterIndex++;
            }

            $functionSig .= ";";

            my $public = ($publicInterfaces{$functionSig} or $newPublicClass);
            delete $publicInterfaces{$functionSig};

            $functionSig .= "\n";

            foreach my $type (keys %typesToForwardDeclare) {
                # add any forward declarations to the public header if a deprecated version will be generated
                AddForwardDeclarationsForType($type, 1) if $needsDeprecatedVersion;
                AddForwardDeclarationsForType($type, $public) unless $public and $needsDeprecatedVersion;
            }

            push(@headerFunctions, $functionSig) if $public;
            push(@privateHeaderFunctions, $functionSig) unless $public;

            # generate the old style method names with un-named parameters, these methods are deprecated
            if ($needsDeprecatedVersion) {
                my $deprecatedFunctionSig = $functionSig;
                $deprecatedFunctionSig =~ s/\s\w+:/ :/g; # remove parameter names
                my $deprecatedFunctionKey = $deprecatedFunctionSig;

                $deprecatedFunctionSig =~ s/;\n$/ DEPRECATED_IN_MAC_OS_X_VERSION_10_5_AND_LATER;\n/ if $buildingForLeopardOrLater;
                push(@deprecatedHeaderFunctions, $deprecatedFunctionSig);

                $deprecatedFunctionKey =~ s/\n$//; # remove the newline
                delete $publicInterfaces{$deprecatedFunctionKey};
            }
        }

        if (@headerFunctions > 0) {
            push(@headerContent, "\n") if $buildingForLeopardOrLater and @headerAttributes > 0;
            push(@headerContent, @headerFunctions);
        }
    }

    # - End @interface 
    push(@headerContent, "\@end\n");

    if (@deprecatedHeaderFunctions > 0) {
        # - Deprecated category @interface 
        push(@headerContent, "\n\@interface $className (" . $className . "Deprecated)\n");
        push(@headerContent, @deprecatedHeaderFunctions);
        push(@headerContent, "\@end\n");
    }

    if (@privateHeaderAttributes > 0 or @privateHeaderFunctions > 0) {
        # - Private category @interface
        @privateHeaderContentHeader = split("\r", $headerLicenceTemplate);

        my $classHeaderName = GetClassHeaderName($className);
        push(@privateHeaderContentHeader, "\n#import <WebCore/$classHeaderName.h>\n\n");

        @privateHeaderContent = ();
        push(@privateHeaderContent, "\@interface $className (" . $className . "Private)\n");
        push(@privateHeaderContent, @privateHeaderAttributes) if @privateHeaderAttributes > 0;
        push(@privateHeaderContent, "\n") if $buildingForLeopardOrLater and @privateHeaderAttributes > 0 and @privateHeaderFunctions > 0;
        push(@privateHeaderContent, @privateHeaderFunctions) if @privateHeaderFunctions > 0;
        push(@privateHeaderContent, "\@end\n");
    }
}

sub GenerateImplementation
{
    my $object = shift;
    my $dataNode = shift;

    my $interfaceName = $dataNode->name;
    my $className = GetClassName($interfaceName);
    my $implClassName = GetImplClassName($interfaceName);
    my $parentImplClassName = GetParentImplClassName($dataNode);

    my $numAttributes = @{$dataNode->attributes};
    my $numFunctions = @{$dataNode->functions};
    my $hasFunctionsOrAttributes = $numAttributes + $numFunctions;

    # - Add default header template.
    @implContentHeader = split("\r", $implementationLicenceTemplate);

    # - INCLUDES -
    push(@implContentHeader, "\n#import \"config.h\"\n");

    my $classHeaderName = GetClassHeaderName($className);
    push(@implContentHeader, "#import \"$classHeaderName.h\"\n\n");

    if ($hasFunctionsOrAttributes) {
        push(@implContentHeader, "#import <wtf/GetPtr.h>\n\n");

        $implIncludes{"$implClassName.h"} = 1;
        $implIncludes{"DOMInternal.h"} = 1;
    }

    @implContent = ();

    # START implementation
    push(@implContent, "\@implementation $className\n\n");

    if ($hasFunctionsOrAttributes) {
        # Add namespace to implementation class name 
        my $implClassNameWithNamespace = "WebCore::" . $implClassName;

        if ($parentImplClassName eq "Object") {
            # Only generate 'dealloc' and 'finalize' methods for direct subclasses of DOMObject.

            push(@implContent, "#define IMPL reinterpret_cast<$implClassNameWithNamespace*>(_internal)\n\n");

            push(@implContent, "- (void)dealloc\n");
            push(@implContent, "{\n");
            push(@implContent, "    if (_internal)\n");
            push(@implContent, "        IMPL->deref();\n");
            push(@implContent, "    [super dealloc];\n");
            push(@implContent, "}\n\n");

            push(@implContent, "- (void)finalize\n");
            push(@implContent, "{\n");
            push(@implContent, "    if (_internal)\n");
            push(@implContent, "        IMPL->deref();\n");
            push(@implContent, "    [super finalize];\n");
            push(@implContent, "}\n\n");
        } elsif ($interfaceName eq "CSSStyleSheet") {
            # Special case for CSSStyleSheet
            push(@implContent, "#define IMPL reinterpret_cast<$implClassNameWithNamespace*>(_internal)\n\n");
        } else {
            my $internalBaseType;
            if ($parentImplClassName eq "CSSValue") {
                $internalBaseType = "WebCore::CSSValue"
            } elsif ($parentImplClassName eq "CSSRule") {
                $internalBaseType = "WebCore::CSSRule"
            } else {
                $internalBaseType = "WebCore::Node"
            }

            push(@implContent, "#define IMPL static_cast<$implClassNameWithNamespace*>(reinterpret_cast<$internalBaseType*>(_internal))\n\n");
        }
    }

    %attributeNames = ();

    # - Attributes
    if ($numAttributes > 0) {
        foreach my $attribute (@{$dataNode->attributes}) {
            AddIncludesForType($attribute->signature->type);

            my $idlType = $codeGenerator->StripModule($attribute->signature->type);

            my $attributeName = $attribute->signature->name;
            my $attributeType = GetObjCType($attribute->signature->type);
            my $attributeIsReadonly = ($attribute->type =~ /^readonly/);

            my $attributeInterfaceName = $attributeName;
            if ($attributeName eq "id") {
                # Special case attribute id to be idName to avoid Obj-C nameing conflict.
                $attributeInterfaceName .= "Name";
            } elsif ($attributeName eq "frame") {
                # Special case attribute frame to be frameBorders.
                $attributeInterfaceName .= "Borders";
            }

            $attributeNames{$attributeInterfaceName} = 1;

            # - GETTER
            my $getterSig = "- ($attributeType)$attributeInterfaceName\n";
            my $hasGetterException = @{$attribute->getterExceptions};
            my $getterContentHead = "IMPL->$attributeName(";
            my $getterContentTail = ")";

            my $attributeTypeSansPtr = $attributeType;
            $attributeTypeSansPtr =~ s/ \*$//; # Remove trailing " *" from pointer types.
            my $typeMaker = GetObjCTypeMaker($attribute->signature->type);

            # Special cases
            if ($attributeName =~ /(\w+)DisplayString$/) {
                my $attributeToDisplay = $1;
                $getterContentHead = "IMPL->$attributeToDisplay().replace(\'\\\\\', [self _element]->document()->backslashAsCurrencySymbol()";
                $implIncludes{"Document.h"} = 1;
            } elsif ($attributeName =~ /^absolute(\w+)URL$/) {
                my $typeOfURL = $1;
                $getterContentHead = "[self _getURLAttribute:";
                if ($typeOfURL eq "Link") {
                    $getterContentTail = "\@\"href\"]";
                } elsif ($typeOfURL eq "Image" and $interfaceName eq "HTMLImageElement") {
                    $getterContentTail = "\@\"src\"]";
                }
                $implIncludes{"DOMPrivate.h"} = 1;
            } elsif ($typeMaker ne "") {
                # Surround getter with TypeMaker
                $getterContentHead = "[$attributeTypeSansPtr $typeMaker:WTF::getPtr(" . $getterContentHead;
                $getterContentTail .= ")]";
            }

            my $getterContent;
            if ($hasGetterException) {
                $getterContent = $getterContentHead . "ec" . $getterContentTail;
            } else {
                $getterContent = $getterContentHead . $getterContentTail;
            }

            push(@implContent, $getterSig);
            push(@implContent, "{\n");
            if ($hasGetterException) {
                # Differentiated between when the return type is a pointer and
                # not for white space issue (ie. Foo *result vs. int result).
                if ($attributeType =~ /\*$/) {
                    $getterContent = $attributeType . "result = " . $getterContent;
                } else {
                    $getterContent = $attributeType . " result = " . $getterContent;
                }

                push(@implContent, "    $exceptionInit\n");
                push(@implContent, "    $getterContent;\n");
                push(@implContent, "    $exceptionRaiseOnError\n");
                push(@implContent, "    return result;\n");
            } else {
                push(@implContent, "    return $getterContent;\n");
            }
            push(@implContent, "}\n\n");

            # - SETTER
            if (!$attributeIsReadonly) {
                # Exception handling
                my $hasSetterException = @{$attribute->setterExceptions};

                $attributeName = "set" . ucfirst($attributeName);
                my $setterName = "set" . ucfirst($attributeInterfaceName);
                my $argName = "new" . ucfirst($attributeInterfaceName);

                # FIXME: should move this out into it's own fuction to share with
                # the similar function parameter code below.
                my $arg = "";
                if ($codeGenerator->IsPrimitiveType($idlType) or $idlType eq "DOMString") {
                    $arg = $argName;
                } elsif ($idlType eq "HTMLTableCaptionElement") {
                    $arg = "[" . $argName . " _tableCaptionElement]";
                } elsif ($idlType eq "HTMLTableSectionElement") {
                    $arg = "[" . $argName . " _tableSectionElement]";
                } else {
                    $arg = "[" . $argName . " _" . lcfirst($idlType) . "]";
                }

                my $setterSig = "- (void)$setterName:($attributeType)$argName\n";

                push(@implContent, $setterSig);
                push(@implContent, "{\n");

                unless ($codeGenerator->IsPrimitiveType($idlType) or $idlType eq "DOMString") {
                    push(@implContent, "    ASSERT($argName);\n\n");
                }

                if ($hasSetterException) {
                    push(@implContent, "    $exceptionInit\n");
                    push(@implContent, "    IMPL->$attributeName($arg, ec);\n");
                    push(@implContent, "    $exceptionRaiseOnError\n");
                } else {
                    push(@implContent, "    IMPL->$attributeName($arg);\n");
                }

                push(@implContent, "}\n\n");
            }
        }
    }

    my @deprecatedFunctions = ();

    # - Functions
    if ($numFunctions > 0) {
        foreach my $function (@{$dataNode->functions}) {
            AddIncludesForType($function->signature->type);

            my $functionName = $function->signature->name;
            my $returnType = GetObjCType($function->signature->type);
            my $hasParameters = @{$function->parameters};
            my $raisesExceptions = @{$function->raisesExceptions};

            my @parameterNames = ();
            my @needsAssert = ();
            my %needsCustom = ();

            my $parameterIndex = 0;
            my $functionSig = "- ($returnType)$functionName";
            foreach my $param (@{$function->parameters}) {
                my $paramName = $param->name;
                my $paramType = GetObjCType($param->type);

                # make a new parameter name if the original conflicts with a property name
                $paramName = "in" . ucfirst($paramName) if $attributeNames{$paramName};

                AddIncludesForType($param->type);

                # FIXME: should move this out into it's own fuction to share with
                # the similar setter parameter code above.
                my $idlType = $codeGenerator->StripModule($param->type);
                if ($codeGenerator->IsPrimitiveType($idlType) or $idlType eq "DOMString") {
                    push(@parameterNames, $paramName);
                } elsif ($idlType eq "XPathNSResolver") {
                    my $implGetter = "[nativeResolver _xpathNSResolver]";
                    push(@parameterNames, $implGetter);
                    $needsCustom{"XPathNSResolver"} = $paramName;
                } elsif ($idlType eq "XPathResult") {
                    my $implGetter = "[" . $paramName . " _xpathResult]";
                    push(@parameterNames, $implGetter);
                } elsif ($idlType eq "HTMLElement") {
                    my $implGetter = "[" . $paramName . " _HTMLElement]";
                    push(@parameterNames, $implGetter);
                } elsif ($idlType eq "HTMLOptionElement") {
                    my $implGetter = "[" . $paramName . " _optionElement]";
                    push(@parameterNames, $implGetter);
                } else {
                    my $implGetter = "[" . $paramName . " _" . lcfirst($idlType) . "]";
                    push(@parameterNames, $implGetter);
                }

                unless ($codeGenerator->IsPrimitiveType($idlType) or $idlType eq "DOMString") {
                    push(@needsAssert, "    ASSERT($paramName);\n");
                }

                if ($parameterIndex >= 1) {
                    my $paramPrefix = $param->extendedAttributes->{"ObjCPrefix"};
                    $paramPrefix = $param->name unless defined($paramPrefix);
                    $functionSig .= " $paramPrefix";
                }

                $functionSig .= ":($paramType)$paramName";

                $parameterIndex++;
            }

            my @functionContent = ();

            # special case the XPathNSResolver
            if (defined $needsCustom{"XPathNSResolver"}) {
                my $paramName = $needsCustom{"XPathNSResolver"};
                push(@functionContent, "    if ($paramName && ![$paramName isMemberOfClass:[DOMNativeXPathNSResolver class]])\n");
                push(@functionContent, "        [NSException raise:NSGenericException format:\@\"createExpression currently does not work with custom NS resolvers\"];\n");
                push(@functionContent, "    DOMNativeXPathNSResolver *nativeResolver = (DOMNativeXPathNSResolver *)$paramName;\n\n");
            }

            push(@parameterNames, "ec") if $raisesExceptions;

            my $content = "IMPL->" . $functionName . "(" . join(", ", @parameterNames) . ")";

            if ($returnType eq "void") {
                # Special case 'void' return type.
                if ($raisesExceptions) {
                    push(@functionContent, "    $exceptionInit\n");
                    push(@functionContent, "    $content;\n");
                    push(@functionContent, "    $exceptionRaiseOnError\n");
                } else {
                    push(@functionContent, "    $content;\n");
                }
            } else {
                my $typeMaker = GetObjCTypeMaker($function->signature->type);
                unless ($typeMaker eq "") {
                    my $returnTypeClass = "";
                    if ($function->signature->type eq "XPathNSResolver") {
                        # Special case XPathNSResolver
                        $returnTypeClass = "DOMNativeXPathNSResolver";
                    } else {
                        # Remove trailing " *" from pointer types.
                        $returnTypeClass = $returnType;
                        $returnTypeClass =~ s/ \*$//;
                    }

                    # Surround getter with TypeMaker
                    if ($returnTypeClass eq "DOMRGBColor") {
                        $content = "[$returnTypeClass $typeMaker:" . $content . "]";
                    } else {
                        $content = "[$returnTypeClass $typeMaker:WTF::getPtr(" . $content . ")]";
                    }
                }

                if ($raisesExceptions) {
                    # Differentiated between when the return type is a pointer and
                    # not for white space issue (ie. Foo *result vs. int result).
                    if ($returnType =~ /\*$/) {
                        $content = $returnType . "result = " . $content;
                    } else {
                        $content = $returnType . " result = " . $content;
                    }

                    push(@functionContent, "    $exceptionInit\n");
                    push(@functionContent, "    $content;\n");
                    push(@functionContent, "    $exceptionRaiseOnError\n");
                    push(@functionContent, "    return result;\n");
                } else {
                    push(@functionContent, "    return $content;\n");
                }
            }

            push(@implContent, "$functionSig\n");
            push(@implContent, "{\n");
            push(@implContent, @functionContent);
            push(@implContent, "}\n\n");

            # generate the old style method names with un-named parameters, these methods are deprecated
            if (@{$function->parameters} > 1 and $function->signature->extendedAttributes->{"OldStyleObjC"}) {
                my $deprecatedFunctionSig = $functionSig;
                $deprecatedFunctionSig =~ s/\s\w+:/ :/g; # remove parameter names

                push(@deprecatedFunctions, "$deprecatedFunctionSig\n");
                push(@deprecatedFunctions, "{\n");
                push(@deprecatedFunctions, @functionContent);
                push(@deprecatedFunctions, "}\n\n");
            }

            # Clear the hash
            %needsCustom = ();
        }
    }

    # END implementation
    push(@implContent, "\@end\n");

    if (@deprecatedFunctions > 0) {
        # - Deprecated category @implementation
        push(@implContent, "\n\@implementation $className (" . $className . "Deprecated)\n\n");
        push(@implContent, @deprecatedFunctions);
        push(@implContent, "\@end\n");
    }
}

# Internal helper
sub WriteData
{
    my $object = shift;
    my $name = shift;

    # Open files for writing...
    my $headerFileName = "$outputDir/" . $name . ".h";
    my $privateHeaderFileName = "$outputDir/" . $name . "Private.h";
    my $implFileName = "$outputDir/" . $name . ".mm";

    # Remove old files.
    unlink($headerFileName);
    unlink($privateHeaderFileName);
    unlink($implFileName);

    # Write implementation file.
    open($IMPL, ">$implFileName") or die "Couldn't open file $implFileName";

    print $IMPL @implContentHeader;

    foreach my $implInclude (sort keys(%implIncludes)) {
        print $IMPL "#import \"$implInclude\"\n";
    }

    print $IMPL "\n" if keys %implIncludes > 0;
    print $IMPL @implContent;

    close($IMPL);
    undef($IMPL);

    undef(@implContentHeader);
    undef(@implContent);
    %implIncludes = ();

    # Write public header.
    open($HEADER, ">$headerFileName") or die "Couldn't open file $headerFileName";
    print $HEADER @headerContentHeader;

    my $forwardDeclarationCount = 0;
    foreach my $forwardClassDeclaration (sort keys(%headerForwardDeclarations)) {
        print $HEADER "\@class $forwardClassDeclaration;\n";
        $forwardDeclarationCount++;
    }

    foreach my $forwardProtocolDeclaration (sort keys(%headerForwardDeclarationsForProtocols)) {
        print $HEADER "\@protocol $forwardProtocolDeclaration;\n";
        $forwardDeclarationCount++;
    }

    print $HEADER "\n" if $forwardDeclarationCount;
    print $HEADER @headerContent;

    close($HEADER);
    undef($HEADER);

    undef(@headerContentHeader);
    undef(@headerContent);
    %headerForwardDeclarations = ();
    %headerForwardDeclarationsForProtocols = ();

    if (@privateHeaderContent > 0) {
        open($PRIVATE_HEADER, ">$privateHeaderFileName") or die "Couldn't open file $privateHeaderFileName";
        print $PRIVATE_HEADER @privateHeaderContentHeader;

        $forwardDeclarationCount = 0;
        foreach my $forwardClassDeclaration (sort keys(%privateHeaderForwardDeclarations)) {
            print $PRIVATE_HEADER "\@class $forwardClassDeclaration;\n";
            $forwardDeclarationCount++;
        }

        foreach my $forwardProtocolDeclaration (sort keys(%privateHeaderForwardDeclarationsForProtocols)) {
            print $PRIVATE_HEADER "\@protocol $forwardProtocolDeclaration;\n";
            $forwardDeclarationCount++;
        }

        print $PRIVATE_HEADER "\n" if $forwardDeclarationCount;
        print $PRIVATE_HEADER @privateHeaderContent;

        close($PRIVATE_HEADER);
        undef($PRIVATE_HEADER);

        undef(@privateHeaderContentHeader);
        undef(@privateHeaderContent);
        %privateHeaderForwardDeclarations = ();
        %privateHeaderForwardDeclarationsForProtocols = ();
    }
}

1;
