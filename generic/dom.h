/*---------------------------------------------------------------------------
|   Copyright (C) 1999  Jochen C. Loewer (loewerj@hotmail.com)
+----------------------------------------------------------------------------
|
|   $Header$
|
|
|   A DOM interface upon the expat XML parser for the C language
|   according to the W3C recommendation REC-DOM-Level-1-19981001
|
|
|   The contents of this file are subject to the Mozilla Public License
|   Version 1.1 (the "License"); you may not use this file except in
|   compliance with the License. You may obtain a copy of the License at
|   http://www.mozilla.org/MPL/
|
|   Software distributed under the License is distributed on an "AS IS"
|   basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
|   License for the specific language governing rights and limitations
|   under the License.
|
|   The Original Code is tDOM.
|
|   The Initial Developer of the Original Code is Jochen Loewer
|   Portions created by Jochen Loewer are Copyright (C) 1998, 1999
|   Jochen Loewer. All Rights Reserved.
|
|   Contributor(s):
|
|
|   $Log$
|   Revision 1.4  2002/03/10 01:14:57  rolf
|   Introduced distinction between XML Name and XML NC Name.
|
|   Revision 1.3  2002/03/07 22:09:46  rolf
|   Added infrastructur to be able to do NCNAME tests.
|   Freeze of actual state, befor feeding stuff to Jochen.
|
|   Revision 1.2  2002/02/24 02:31:27  rolf
|   Fixed UTF-8 char byte length determination
|
|   Revision 1.1.1.1  2002/02/22 01:05:35  rolf
|   tDOM0.7test with Jochens first set of patches
|
|
|
|   written by Jochen Loewer
|   April 5, 1999
|
\--------------------------------------------------------------------------*/

#ifndef __DOM_H__
#define __DOM_H__


#include <tcl.h>
/*#include <xmlparse.h> */
#include <expat.h> 
#include <utf8conv.h>



/*--------------------------------------------------------------------------
|   Global DOM node/document object counters and tag/attribute hash tables.
|
|   Encapsulated within a structure which, if used in thread environment,
|   will be stored in thread local storage.
|
\-------------------------------------------------------------------------*/
#ifdef TCL_THREADS

   typedef struct TDomGlobalThreadSpecificData {
  
       unsigned int  domModuleIsInitialized;
       unsigned int  domUniqueNodeNr;
       unsigned int  domUniqueDocNr;
       Tcl_HashTable tagNames;
       Tcl_HashTable attrNames;
      
   } TDomGlobalThreadSpecificData;
  
   extern Tcl_ThreadDataKey  domDataKey;
#  define TSDPTR(x)          tsdPtr->x
#  define TDomThreaded(x)    x
#  define GetTDomTSD()       TDomGlobalThreadSpecificData *tsdPtr = \
                                 (TDomGlobalThreadSpecificData*)    \
                                 Tcl_GetThreadData( &domDataKey,    \
                                 sizeof(TdomGlobalThreadSpecificData) )

  
#else

  extern unsigned int  domUniqueNodeNr;
  extern unsigned int  domUniqueDocNr;
  extern Tcl_HashTable tagNames;
  extern Tcl_HashTable attrNames;     
  
#  define TSDPTR(x)          x
#  define TDomThreaded(x)     
#  define GetTDomTSD()       

  /*
  define Tcl_Mutex char
  define Tcl_MutexLock(a)
  define Tcl_MutexUnlock(a)
  define GLOBAL_TSD_KEY(a)  a
  */
  
#endif

#if (TCL_MAJOR_VERSION == 8 && TCL_MINOR_VERSION == 0) || TCL_MAJOR_VERSION < 8 
#define TclOnly8Bits 1
#else
#define TclOnly8Bits 0
#endif

#define UTF8_1BYTE_CHAR(c) ( 0    == ((c) & 0x80))
#define UTF8_2BYTE_CHAR(c) ( 0xC0 == ((c) & 0xE0))
#define UTF8_3BYTE_CHAR(c) ( 0xE0 == ((c) & 0xF0))
#define UTF8_4BYTE_CHAR(c) ( 0xF0 == ((c) & 0xF8))

#if TclOnly8Bits
#define UTF8_CHAR_LEN(c) 1
#else 
#define UTF8_CHAR_LEN(c) \
  UTF8_1BYTE_CHAR((c)) ? 1 : \
   (UTF8_2BYTE_CHAR((c)) ? 2 : \
     (UTF8_3BYTE_CHAR((c)) ? 3 : 0))
#endif

/* The following 2 defines are out of the expat code */

/* A 2 byte UTF-8 representation splits the characters 11 bits
between the bottom 5 and 6 bits of the bytes.
We need 8 bits to index into pages, 3 bits to add to that index and
5 bits to generate the mask. */
#define UTF8_GET_NAMING2(pages, byte) \
    (namingBitmap[((pages)[(((byte)[0]) >> 2) & 7] << 3) \
                      + ((((byte)[0]) & 3) << 1) \
                      + ((((byte)[1]) >> 5) & 1)] \
         & (1 << (((byte)[1]) & 0x1F)))

/* A 3 byte UTF-8 representation splits the characters 16 bits
between the bottom 4, 6 and 6 bits of the bytes.
We need 8 bits to index into pages, 3 bits to add to that index and
5 bits to generate the mask. */
#define UTF8_GET_NAMING3(pages, byte) \
  (namingBitmap[((pages)[((((byte)[0]) & 0xF) << 4) \
                             + ((((byte)[1]) >> 2) & 0xF)] \
                       << 3) \
                      + ((((byte)[1]) & 3) << 1) \
                      + ((((byte)[2]) >> 5) & 1)] \
         & (1 << (((byte)[2]) & 0x1F)))

#define UTF8_GET_NAMING_NMTOKEN(p, n) \
  ((n) == 1 \
  ? nameChar7Bit[(int)(*(p))] \
  : ((n) == 2 \
    ? UTF8_GET_NAMING2(nmstrtPages, (const unsigned char *)(p)) \
    : ((n) == 3 \
      ? UTF8_GET_NAMING3(nmstrtPages, (const unsigned char *)(p)) \
      : 0)))

#define UTF8_GET_NAMING_NCNMTOKEN(p, n) \
  ((n) == 1 \
  ? NCnameChar7Bit[(int)(*(p))] \
  : ((n) == 2 \
    ? UTF8_GET_NAMING2(nmstrtPages, (const unsigned char *)(p)) \
    : ((n) == 3 \
      ? UTF8_GET_NAMING3(nmstrtPages, (const unsigned char *)(p)) \
      : 0)))


#define UTF8_GET_NAMING_NAME(p, n) \
  ((n) == 1 \
  ? nameStart7Bit[(int)(*(p))] \
  : ((n) == 2 \
    ? UTF8_GET_NAMING2(namePages, (const unsigned char *)(p)) \
    : ((n) == 3 \
      ? UTF8_GET_NAMING3(namePages, (const unsigned char *)(p)) \
      : 0)))

#define UTF8_GET_NAMING_NCNAME(p, n) \
  ((n) == 1 \
  ? NCnameStart7Bit[(int)(*(p))] \
  : ((n) == 2 \
    ? UTF8_GET_NAMING2(namePages, (const unsigned char *)(p)) \
    : ((n) == 3 \
      ? UTF8_GET_NAMING3(namePages, (const unsigned char *)(p)) \
      : 0)))


#include "../expat-1.95.1/nametab.h"

static const unsigned char nameChar7Bit[] = {
/* 0x00 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x08 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x10 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x18 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x20 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x28 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00,
/* 0x30 */    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
/* 0x38 */    0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x40 */    0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
/* 0x48 */    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
/* 0x50 */    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
/* 0x58 */    0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01,
/* 0x60 */    0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
/* 0x68 */    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
/* 0x70 */    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
/* 0x78 */    0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const unsigned char NCnameChar7Bit[] = {
/* 0x00 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x08 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x10 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x18 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x20 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x28 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00,
/* 0x30 */    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
/* 0x38 */    0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x40 */    0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
/* 0x48 */    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
/* 0x50 */    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
/* 0x58 */    0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01,
/* 0x60 */    0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
/* 0x68 */    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
/* 0x70 */    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
/* 0x78 */    0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
};


static const unsigned char nameStart7Bit[] = {
/* 0x00 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x08 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x10 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x18 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x20 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x28 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x30 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x38 */    0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x40 */    0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
/* 0x48 */    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
/* 0x50 */    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
/* 0x58 */    0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01,
/* 0x60 */    0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
/* 0x68 */    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
/* 0x70 */    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
/* 0x78 */    0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
};


static const unsigned char NCnameStart7Bit[] = {
/* 0x00 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x08 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x10 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x18 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x20 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x28 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x30 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x38 */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x40 */    0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
/* 0x48 */    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
/* 0x50 */    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
/* 0x58 */    0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01,
/* 0x60 */    0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
/* 0x68 */    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
/* 0x70 */    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
/* 0x78 */    0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
};


#if TclOnly8Bits == 1
#  define isNameStart(x) (isalpha(*x) || ((*x)=='_') || ((*x)==':'))
#  define isNameChar(x) (isalnum(*x)  || ((*x)=='_') || ((*x)=='-') || ((*x)=='.') || ((*x)==':'))
#  define isNCNameStart(x) (isalpha(*x) || ((*x)=='_'))
#  define isNCNameChar(x) (isalnum(*x)  || ((*x)=='_') || ((*x)=='-') || ((*x)=='.'))
#else 
static int isNameStart(char *c) 
{
    int clen;
    clen = UTF8_CHAR_LEN (*c);
    return (UTF8_GET_NAMING_NAME(c, clen));
}
static int isNCNameStart(char *c) 
{
    int clen;
    clen = UTF8_CHAR_LEN (*c);
    return (UTF8_GET_NAMING_NCNAME(c, clen));
}
static int isNameChar(char *c)
{
    int clen;
    clen = UTF8_CHAR_LEN (*c);
    return (UTF8_GET_NAMING_NMTOKEN(c, clen));
}
static int isNCNameChar(char *c)
{
    int clen;
    clen = UTF8_CHAR_LEN (*c);
    return (UTF8_GET_NAMING_NCNMTOKEN(c, clen));
}
#endif

/*--------------------------------------------------------------------------
|   DOMString
|
\-------------------------------------------------------------------------*/
typedef char* domString;   /* should 16-bit unicode character !!*/


/*--------------------------------------------------------------------------
|   DOM_nodeType
|
\-------------------------------------------------------------------------*/
typedef enum {

    ELEMENT_NODE                = 1,
    ATTRIBUTE_NODE              = 2,
    TEXT_NODE                   = 3,
    CDATA_SECTION_NODE          = 4,
    ENTITY_REFERENCE_NODE       = 5,
    ENTITY_NODE                 = 6,
    PROCESSING_INSTRUCTION_NODE = 7,
    COMMENT_NODE                = 8,
    DOCUMENT_NODE               = 9, 
    DOCUMENT_TYPE_NODE          = 10,
    DOCUMENT_FRAGMENT_NODE      = 11,
    NOTATION_NODE               = 12,
    ALL_NODES                   = 100 
} domNodeType;


/*--------------------------------------------------------------------------
|   flags   -  indicating some internal features about nodes
|
\-------------------------------------------------------------------------*/
typedef int domNodeFlags;

#define HAS_LINE_COLUMN           1     
#define VISIBLE_IN_TCL            2
#define IS_ID_ATTRIBUTE           4
#define HAS_BASEURI               8
#define DISABLE_OUTPUT_ESCAPING  16
#define HAS_NS_INFO              32

typedef int domDocFlags;

#define OUTPUT_DEFAULT_XML        1
#define OUTPUT_DEFAULT_HTML       2
#define OUTPUT_DEFAULT_TEXT       4
#define OUTPUT_DEFAULT_UNKOWN     8
#define USE_8_BIT_ENCODING       16

/*--------------------------------------------------------------------------
|   a index to the namespace records
|
\-------------------------------------------------------------------------*/
typedef int domNameSpaceIndex;



/*--------------------------------------------------------------------------
|   domException
|
\-------------------------------------------------------------------------*/
typedef enum {

    OK                          = 0,
    INDEX_SIZE_ERR              = 1,
    DOMSTRING_SIZE_ERR          = 2,
    HIERARCHY_REQUEST_ERR       = 3,
    WRONG_DOCUMENT_ERR          = 4,
    INVALID_CHARACTER_ERR       = 5,
    NO_DATA_ALLOWED_ERR         = 6,
    NO_MODIFICATION_ALLOWED_ERR = 7,
    NOT_FOUND_ERR               = 8,
    NOT_SUPPORTED_ERR           = 9,
    INUSE_ATTRIBUTE_ERR         = 10
    
} domException;



/*--------------------------------------------------------------------------
|   DOM_Document
|
\-------------------------------------------------------------------------*/
typedef struct domDocument {

    domNodeType       nodeType  : 8;
    domDocFlags       nodeFlags : 8;
    domNameSpaceIndex dummy     : 16;
    unsigned int      documentNumber;        
    struct domNode   *documentElement;
    struct domNode   *fragments;
    int               nsCount;
    struct domNS     *namespaces;
    struct domNode   *rootNode;
    Tcl_HashTable    *ids;
    Tcl_HashTable    *unparsedEntities;
    Tcl_HashTable    *baseURIs;
    Tcl_HashTable    *NSscopes;
    Tcl_Obj          *extResolver;
} domDocument;


/*--------------------------------------------------------------------------
|   namespace
|
\-------------------------------------------------------------------------*/
typedef struct domNS {

   int           index;
   char         *uri;
   char         *prefix;
   int           used;
   struct domNS *next;

} domNS;

#define MAX_PREFIX_LEN   80



/*--------------------------------------------------------------------------
|   domLineColumn
|
\-------------------------------------------------------------------------*/
typedef struct domLineColumn {

    int   line;
    int   column;

} domLineColumn;
        

/*--------------------------------------------------------------------------
|   domNode
|
\-------------------------------------------------------------------------*/
typedef struct domNode {

    domNodeType         nodeType  : 8;
    domNodeFlags        nodeFlags : 8;
    domNameSpaceIndex   namespace : 8;
    int                 info      : 8;
    unsigned int        nodeNumber; 
    domDocument        *ownerDocument;
    struct domNode     *parentNode;
    struct domNode     *previousSibling;
    struct domNode     *nextSibling;

    domString           nodeName;  /* now the element node specific fields */
    struct domNode     *firstChild;
    struct domNode     *lastChild;
    struct domAttrNode *firstAttr;
    
} domNode;



/*--------------------------------------------------------------------------
|   domTextNode
|
\-------------------------------------------------------------------------*/
typedef struct domTextNode {

    domNodeType         nodeType  : 8;
    domNodeFlags        nodeFlags : 8;
    domNameSpaceIndex   namespace : 8;
    int                 info      : 8;
    unsigned int        nodeNumber; 
    domDocument        *ownerDocument;
    struct domNode     *parentNode;
    struct domNode     *previousSibling;
    struct domNode     *nextSibling;
    
    domString           nodeValue;   /* now the text node specific fields */
    int                 valueLength;

} domTextNode;


/*--------------------------------------------------------------------------
|   domProcessingInstructionNode
|
\-------------------------------------------------------------------------*/
typedef struct domProcessingInstructionNode {

    domNodeType         nodeType  : 8;
    domNodeFlags        nodeFlags : 8;
    domNameSpaceIndex   namespace : 8;
    int                 info      : 8;
    unsigned int        nodeNumber; 
    domDocument        *ownerDocument;
    struct domNode     *parentNode;
    struct domNode     *previousSibling;
    struct domNode     *nextSibling;
    
    domString           targetValue;   /* now the pi specific fields */
    int                 targetLength;
    domString           dataValue;
    int                 dataLength;

} domProcessingInstructionNode;


/*--------------------------------------------------------------------------
|   domAttrNode
|
\-------------------------------------------------------------------------*/
typedef struct domAttrNode {
 
    domNodeType         nodeType  : 8;
    domNodeFlags        nodeFlags : 8;
    domNameSpaceIndex   namespace : 8;
    int                 info      : 8;
    domString           nodeName;
    domString           nodeValue;
    int                 valueLength;
    struct domNode     *parentNode;
    struct domAttrNode *nextSibling;

} domAttrNode;




/*--------------------------------------------------------------------------
|   domAddCallback
|
\-------------------------------------------------------------------------*/
typedef int  (*domAddCallback)  (domNode * node, void * clientData);
typedef void (*domFreeCallback) (domNode * node, void * clientData);
                                                                                            


/*--------------------------------------------------------------------------
|   Function prototypes
|
\-------------------------------------------------------------------------*/
char *         domException2String (domException expection);

void           domModuleInitialize (void);
domDocument *  domCreateDoc ();
domDocument *  domCreateDocument (Tcl_Interp *interp,
                                  char *documentElementTagName,
                                  char *uri);
   
domDocument *  domReadDocument (XML_Parser parser, 
                                char *xml, 
                                int   length, 
                                int   ignoreWhiteSpaces,
                                TEncoding *encoding_8bit,
                                int   storeLineColumn,
                                int   feedbackAfter,
                                Tcl_Channel channel,
                                char *baseurl,
                                Tcl_Obj *extResolver,
                                Tcl_Interp *interp);
                                
void           domFreeDocument (domDocument *doc, domFreeCallback freeCB, void * clientData);
void           domFreeNode (domNode *node, domFreeCallback freeCB, void *clientData);
            
domTextNode *  domNewTextNode (domDocument *doc,
                               char        *value,
                               int          length,
                               domNodeType  nodeType);

domNode *      domNewElementNode (domDocument *doc,
                                  char        *tagName,
				  domNodeType nodeType);
				  
domNode *      domNewElementNodeNS (domDocument *doc,
                                    char        *tagName,
                                    char        *uri,
				    domNodeType nodeType);

domProcessingInstructionNode * domNewProcessingInstructionNode (
                                  domDocument *doc,
                                  char        *targetValue,
                                  int          targetLength,
                                  char        *dataValue,
                                  int          dataLength);

domAttrNode *  domSetAttribute (domNode *node, char *attributeName,
                                               char *attributeValue);

domAttrNode *  domSetAttributeNS (domNode *node, char *attributeName,
                                                 char *attributeValue,
                                                 char *uri);


int            domRemoveAttribute (domNode *node, char *attributeName);
int            domRemoveAttributeNS (domNode *node, char *uri, char *localName);
domException   domDeleteNode   (domNode *node, domFreeCallback freeCB, void *clientData);
domException   domRemoveChild  (domNode *node, domNode *childToRemove);
domException   domAppendChild  (domNode *node, domNode *childToAppend);
domException   domInsertBefore (domNode *node, domNode *childToInsert, domNode *refChild);
domException   domReplaceChild (domNode *node, domNode *newChild, domNode *oldChild);
domException   domSetNodeValue (domNode *node, char *nodeValue, int valueLen);
domNode *      domCloneNode (domNode *node, int deep);

domTextNode *  domAppendNewTextNode (domNode *parent, char *value, int length, domNodeType nodeType, int disableOutputEscaping);
domNode *      domAppendNewElementNode (domNode *parent, char *tagName, char *uri);

char *         domNamespacePrefix (domNode *node);
char *         domNamespaceURI    (domNode *node);
char *         domGetLocalName    (char *nodeName);
int            domSplitQName (char *name, char *prefix, char **localName);
domNS *        domLookupNamespace (domDocument *doc, char *prefix, char *namespaceURI);
domNS *        domLookupPrefix  (domNode *node, char *prefix);
domNS *        domLookupURI     (domNode *node, char *uri);
domNS *        domGetNamespaceByIndex (domDocument *doc, int nsIndex);
int            domGetLineColumn (domNode *node, int *line, int *column);

int            domXPointerChild (domNode * node, int all, int instance, domNodeType type,
                                 char *element, char *attrName, char *attrValue,
                                 int attrLen, domAddCallback addCallback, 
                                 void * clientData);

int            domXPointerDescendant (domNode * node, int all, int instance, 
                                      int * i, domNodeType type, char *element, 
                                      char *attrName, char *attrValue, int attrLen,
                                      domAddCallback addCallback, void * clientData);

int            domXPointerAncestor (domNode * node, int all, int instance, 
                                    int * i, domNodeType type, char *element, 
                                    char *attrName, char *attrValue, int attrLen,
                                    domAddCallback addCallback, void * clientData);

int            domXPointerXSibling (domNode * node, int forward_mode, int all, int instance, 
                                    domNodeType type, char *element, char *attrName, 
                                    char *attrValue, int attrLen,
                                    domAddCallback addCallback, void * clientData);

char *         findBaseURI (domNode *node);

void           tcldom_tolower (char *str, char *str_out, int  len);


/*---------------------------------------------------------------------------
|   coercion routines for calling from C++
|
\--------------------------------------------------------------------------*/ 
domAttrNode                  * coerceToAttrNode( domNode *n );
domTextNode                  * coerceToTextNode( domNode *n );
domProcessingInstructionNode * coerceToProcessingInstructionNode( domNode *n );


#endif
