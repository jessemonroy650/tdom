#----------------------------------------------------------------------------
#   Copyright (c) 1999 Jochen Loewer (loewerj@hotmail.com)
#----------------------------------------------------------------------------
#
#   $Header$
#
#
#   The higher level functions of tDOM written in plain Tcl.
#
#
#   The contents of this file are subject to the Mozilla Public License
#   Version 1.1 (the "License"); you may not use this file except in
#   compliance with the License. You may obtain a copy of the License at
#   http://www.mozilla.org/MPL/
#
#   Software distributed under the License is distributed on an "AS IS"
#   basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
#   License for the specific language governing rights and limitations
#   under the License.
#
#   The Original Code is tDOM.
#
#   The Initial Developer of the Original Code is Jochen Loewer
#   Portions created by Jochen Loewer are Copyright (C) 1998, 1999
#   Jochen Loewer. All Rights Reserved.
#
#   Contributor(s):
#       Rolf Ade (rolf@pointsman.de):   'fake' nodelists/live childNodes
#
#
#   $Log$
#   Revision 1.3  2002/02/28 00:39:00  rolf
#   Added tcl coded xpath function element-avaliable. Changed
#   function-avaliable accordingly.
#
#   Revision 1.2  2002/02/26 14:04:16  rolf
#   Updated the [package provide] to the new version number 0.7
#
#   Revision 1.1.1.1  2002/02/22 01:05:35  rolf
#   tDOM0.7test with Jochens first set of patches
#
#
#
#   written by Jochen Loewer
#   April, 1999
#
#----------------------------------------------------------------------------


package provide tdom 0.7


#----------------------------------------------------------------------------
#   setup namespaces for additional Tcl level methods, etc.
#
#----------------------------------------------------------------------------
namespace eval ::dom {
    namespace eval  domDoc {
    }
    namespace eval  domNode {
    }
    namespace eval  DOMImplementation {
    }
    namespace eval  xpathFunc {
    }
    namespace eval  xpathFuncHelper {
    }
}



#----------------------------------------------------------------------------
#   hasFeature (DOMImplementation method)
#
#
#   @in  url    the URL, where to get the XML document
#
#   @return     document object
#   @exception  XML parse errors, ...
#
#----------------------------------------------------------------------------
proc ::dom::DOMImplementation::hasFeature { dom feature {version ""} } {

    switch $feature {
        xml -
        XML {
            if {($version == "") || ($version == "1.0")} {
                return 1
            }
        }
    }
    return 0

}

#----------------------------------------------------------------------------
#   load (DOMImplementation method)
#
#       requests a XML document via http using the given URL and
#       builds up a DOM tree in memory returning the document object
#
#
#   @in  url    the URL, where to get the XML document
#
#   @return     document object
#   @exception  XML parse errors, ...
#
#----------------------------------------------------------------------------
proc ::dom::DOMImplementation::load { dom url } {

    error "Sorry, load method not implemented yet!"

}




#----------------------------------------------------------------------------
#   isa (docDoc method, for [incr tcl] compatibility)
#
#
#   @in  className
#
#   @return         1 iff inherits from the given class
#
#----------------------------------------------------------------------------
proc ::dom::domDoc::isa { doc className } {

    if {$className == "domDoc"} {
        return 1
    }
    return 0
}

#----------------------------------------------------------------------------
#   info (domDoc method, for [incr tcl] compatibility)
#
#
#   @in  subcommand
#   @in  args
#
#----------------------------------------------------------------------------
proc ::dom::domDoc::info { doc subcommand args } {

    switch $subcommand {
        class {
            return "domDoc"
        }
        inherit {
            return ""
        }
        heritage {
            return "domDoc {}"
        }
        default {
            error "domDoc::info subcommand $subcommand not yet implemented!"
        }
    }
}

#----------------------------------------------------------------------------
#   importNode (domDoc method)
#
#       Document Object Model (Core) Level 2 method
#
#
#   @in  subcommand
#   @in  args
#
#----------------------------------------------------------------------------
proc ::dom::domDoc::importNode { doc importedNode deep } {

    if {$deep || ($deep == "-deep"} {
        set node [$importedNode cloneNode -deep]
    } else {
        set node [$importedNode cloneNode]
    }
    return $node
}






#----------------------------------------------------------------------------
#   isa (domNode method, for [incr tcl] compatibility)
#
#
#   @in  className
#
#   @return         1 iff inherits from the given class
#
#----------------------------------------------------------------------------
proc ::dom::domNode::isa { doc className } {

    if {$className == "domNode"} {
        return 1
    }
    return 0
}

#----------------------------------------------------------------------------
#   info (domNode method, for [incr tcl] compatibility)
#
#
#   @in  subcommand
#   @in  args
#
#----------------------------------------------------------------------------
proc ::dom::domNode::info { doc subcommand args } {

    switch $subcommand {
        class {
            return "domNode"
        }
        inherit {
            return ""
        }
        heritage {
            return "domNode {}"
        }
        default {
            error "domNode::info subcommand $subcommand not yet implemented!"
        }
    }
}

#----------------------------------------------------------------------------
#   isWithin (domNode method)
#
#       tests, whether a node object is nested below another tag
#
#
#   @in  tagName  the nodeName of an elment node
#
#   @return       1 iff node is nested below a element with nodeName tagName
#                 0 otherwise
#
#----------------------------------------------------------------------------
proc ::dom::domNode::isWithin { node tagName } {

    while {[$node parentNode] != ""} {
        set node [$node parentNode]
        if {[$node nodeName] == $tagName} {
            return 1
        }
    }
    return 0
}


#----------------------------------------------------------------------------
#   tagName (domNode method)
#
#       same a nodeName for element interface
#
#----------------------------------------------------------------------------
proc ::dom::domNode::tagName { node } {

    if {[$node nodeType] == "ELEMENT_NODE"} {
        return [$node nodeName]
    }
    return -code error "NOT_SUPPORTED_ERR not an element!"
}


#----------------------------------------------------------------------------
#   simpleTranslate (domNode method)
#
#       applies simple translation rules similar to Cost's simple
#       translations to a node
#
#
#   @in  output_var
#   @in  trans_specs
#
#----------------------------------------------------------------------------
proc ::dom::domNode::simpleTranslate { node output_var trans_specs } {

    upvar $output_var output

    if {[$node nodeType] == "TEXT_NODE"} {
        append output [cgiQuote [$node nodeValue]]
        return
    }
    set found 0

    foreach {match action} $trans_specs {

        if {[catch {
            if {!$found && ([$node selectNode self::$match] != "") } {
              set found 1
            }
        } err]} {
            if {![string match "NodeSet expected for parent axis!" $err]} {
                error $err
            }
        }
        if {$found && ($action != "-")} {
            set stop 0
            foreach {type value} $action {
                switch $type {
                    prefix { append output [subst $value] }
                    tag    { append output <$value>       }
                    start  { append output [eval $value]  }
                    stop   { set stop 1                   }
                }
            }
            if {!$stop} {
                foreach child [$node childNodes] {
                    simpleTranslate  $child output $trans_specs
                }
            }
            foreach {type value} $action {
                switch $type {
                    suffix { append output [subst $value] }
                    end    { append output [eval $value]  }
                    tag    { append output </$value>      }
                }
            }
            return
        }
    }
    foreach child [$node childNodes] {
        simpleTranslate $child output $trans_specs
    }
}


#----------------------------------------------------------------------------
#   transfromNode (domNode method)
#
#       applies an XSL stylesheet to the subtree root at the given
#       node object
#
#
#   @in  xsl_doc     document object to a XSL style sheet document
#   @return
#
#----------------------------------------------------------------------------
proc ::dom::domNode::transfromNode { node xsl_doc } {

    error "transfromNode (XSL transformation) is not implemented yet!"
}


#----------------------------------------------------------------------------
#   a DOM conformant 'live' childNodes
#
#       suggested by Rolf Ade (rolf@pointsman.de)
#
#   @return   a 'nodelist' object (it is just the normal node)
#
#----------------------------------------------------------------------------
proc ::dom::domNode::childNodesLive { node } {

    return $node
}


#----------------------------------------------------------------------------
#   item method on a 'nodelist' object
#
#       suggested by Rolf Ade (rolf@pointsman.de)
#
#   @return   a 'nodelist' object (it is just a normal
#
#----------------------------------------------------------------------------
proc ::dom::domNode::item { nodeListNode index } {

    return [lindex [$nodeListNode childNodes] $index]
}


#----------------------------------------------------------------------------
#   length method on a 'nodelist' object
#
#       suggested by Rolf Ade (rolf@pointsman.de)
#
#   @return   a 'nodelist' object (it is just a normal
#
#----------------------------------------------------------------------------
proc ::dom::domNode::length { nodeListNode } {

    return [llength [$nodeListNode childNodes] $childNodes]
}


#----------------------------------------------------------------------------
#   appendData on a 'CharacterData' object
#
#----------------------------------------------------------------------------
proc ::dom::domNode::appendData { node  arg } {

    set type [$node nodeType]
    if {($type != "TEXT_NODE") && ($type != "CDATA_SECTION_NODE") &&
        ($type != "COMMENT_NODE")
    } {
        return -code error "NOT_SUPPORTED_ERR: node is not a cdata node"
    }
    set oldValue [$node nodeValue]
    $node nodeValue [append oldValue $arg]
}

#----------------------------------------------------------------------------
#   deleteData on a 'CharacterData' object
#
#----------------------------------------------------------------------------
proc ::dom::domNode::deleteData { node offset count } {

    set type [$node nodeType]
    if {($type != "TEXT_NODE") && ($type != "CDATA_SECTION_NODE") &&
        ($type != "COMMENT_NODE")
    } {
        return -code error "NOT_SUPPORTED_ERR: node is not a cdata node"
    }
    incr offset -1
    set before [string range [$node nodeValue] 0 $offset]
    incr offset
    incr offset $count
    set after  [string range [$node nodeValue] $offset end]
    $node nodeValue [append before $after]
}

#----------------------------------------------------------------------------
#   insertData on a 'CharacterData' object
#
#----------------------------------------------------------------------------
proc ::dom::domNode::insertData { node  offset arg } {

    set type [$node nodeType]
    if {($type != "TEXT_NODE") && ($type != "CDATA_SECTION_NODE") &&
        ($type != "COMMENT_NODE")
    } {
        return -code error "NOT_SUPPORTED_ERR: node is not a cdata node"
    }
    incr offset -1
    set before [string range [$node nodeValue] 0 $offset]
    incr offset
    set after  [string range [$node nodeValue] $offset end]
    $node nodeValue [append before $arg $after]
}

#----------------------------------------------------------------------------
#   replaceData on a 'CharacterData' object
#
#----------------------------------------------------------------------------
proc ::dom::domNode::replaceData { node offset count arg } {

    set type [$node nodeType]
    if {($type != "TEXT_NODE") && ($type != "CDATA_SECTION_NODE") &&
        ($type != "COMMENT_NODE")
    } {
        return -code error "NOT_SUPPORTED_ERR: node is not a cdata node"
    }
    incr offset -1
    set before [string range [$node nodeValue] 0 $offset]
    incr offset
    incr offset $count
    set after  [string range [$node nodeValue] $offset end]
    $node nodeValue [append before $arg $after]
}

#----------------------------------------------------------------------------
#   substringData on a 'CharacterData' object
#
#   @return   part of the node value (text)
#
#----------------------------------------------------------------------------
proc ::dom::domNode::substringData { node offset count } {

    set type [$node nodeType]
    if {($type != "TEXT_NODE") && ($type != "CDATA_SECTION_NODE") &&
        ($type != "COMMENT_NODE")
    } {
        return -code error "NOT_SUPPORTED_ERR: node is not a cdata node"
    }
    set endOffset [expr $offset + $count - 1]
    return [string range [$node nodeValue] $offset $endOffset]
}


proc ::dom::xpathFuncHelper::coerce2number { type value } {
    switch $type {
        empty      { return 0 }
        number -
        string     { return $value }
        attrvalues { return [lindex $value 0] }
        nodes      { return [[lindex $value 0] selectNodes number()] }
        attrnodes  { return [lindex $value 1] }
    }
}

proc ::dom::xpathFuncHelper::coerce2string { type value } {
    switch $type {
        empty      { return 0 }
        number -
        string     { return $value }
        attrvalues { return [lindex $value 0] }
        nodes      { return [[lindex $value 0] selectNodes string()] }
        attrnodes  { return [lindex $value 1] }
    }
}

#----------------------------------------------------------------------------
#   functions-available
#
#----------------------------------------------------------------------------
proc ::dom::xpathFunc::function-available { ctxNode pos
                                            nodeListType nodeList args} {

    if {[llength $args] != 2} {
        error "function-available(); wrong # of args!"
    }
    foreach { arg1Typ arg1Value } $args break
    set str [::dom::xpathFuncHelper::coerce2string $arg1Typ $arg1Value ]
    switch $str {
        boolean -
        ceiling -
        concat -
        contains -
        count -
        current -
        document -
        element-avaliable -
        false -
        floor -
        generate-id -
        id -
        key -
        last -
        lang -
        local-name -
        name -
        namespace-uri -
        normalize-space -
        not -
        number -
        position -
        round -
        starts-with -
        string -
        string-length -
        substring -
        substring-after -
        substring-before -
        sum -
        translate -
        true -
        unparsed-entity-uri {
            return [list bool true]
        }
        format-number {
            return [list bool false]
        }
        default {
            set TclXpathFuncs [info procs ::dom::xpathFunc::*]
            if {[lsearch -exact $TclXpathFuncs $str] != -1} {
                return [list bool true]
            } else {
                return [list bool false]
            }
        }
    }
}

#----------------------------------------------------------------------------
#   element-avaliable
#
#----------------------------------------------------------------------------
proc ::dom::xpathFunc::element-avaliable { ctxNode pos
                                            nodeListType nodeList args} {

    if {[llength $args] != 2} {
        error "element-avaliable(); wrong # of args!"
    }
    foreach { arg1Typ arg1Value } $args break
    set str [::dom::xpathFuncHelper::coerce2string $arg1Typ $arg1Value ]
    switch $str {
        stylesheet -
        transform -
        include -
        import -
        strip-space -
        preserve-space -
        template -
        apply-templates -
        apply-imports -
        call-template -
        element -
        attribute -
        attribute-set -
        text -
        processing-instruction -
        comment -
        copy -
        value-of -
        number -
        for-each -
        if -
        choose -
        when -
        otherwise -
        sort -
        variable -
        param -
        copy-of -
        with-param -
        key -
        message {
            return [list bool true]
        }
        decimal-format -
        output -
        namespace-alias -
        fallback -
        default {
            return [list bool false]
        }
    }
}


#----------------------------------------------------------------------------
#   system-property
#
#----------------------------------------------------------------------------
proc ::dom::xpathFunc::system-property { ctxNode pos
                                         nodeListType nodeList args } {

    if {[llength $args] != 2} {
        error "system-property(): wrong # of args!"
    }
    foreach { arg1Typ arg1Value } $args break
    set str [::dom::xpathFuncHelper::coerce2string $arg1Typ $arg1Value ]
    switch $str {
        xsl:version {
            return [list number 1.0]
        }
        xsl:vendor {
            return [list string "Jochen Loewer et. al. (loewerj@hotmail.com)"]
        }
        xsl:vendor-url {
            return [list string "http://sdf.lonestar.org/~loewerj/tdom.cgi"]
        }
        default {
            return [list string ""]
        }
    }
}