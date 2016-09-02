// File MCORE/MXmlDocument.cpp

#include "MCOREExtern.h"

#if !M_NO_XML

#define M__MCORE_MXMLDOCUMENT_COMPILING

#include "MXmlDocument.h"
#include "MException.h"
#include "MUtilities.h"
#include "MStreamFile.h"
#include "MStreamMemory.h"
#include "MStr.h"

#include "private/pugixml.cxx"

M_START_PROPERTIES(XmlNode)
   M_CLASS_ENUMERATION                         (XmlNode, NodeDocument)
   M_CLASS_ENUMERATION                         (XmlNode, NodeElement)
   M_CLASS_ENUMERATION                         (XmlNode, NodePcdata)
   M_CLASS_ENUMERATION                         (XmlNode, NodeCdata)
   M_CLASS_ENUMERATION                         (XmlNode, NodeComment)
   M_CLASS_ENUMERATION                         (XmlNode, NodePi)
   M_CLASS_ENUMERATION                         (XmlNode, NodeDeclaration)
   M_CLASS_ENUMERATION                         (XmlNode, NodeDoctype)
   M_OBJECT_PROPERTY_READONLY_INT              (XmlNode, NodeType)
   M_OBJECT_PROPERTY_READONLY_STRING_EXACT     (XmlNode, AsString,   ST_MStdString_X)
   M_OBJECT_PROPERTY_STRING                    (XmlNode, Name,       ST_MStdString_X, ST_X_constMStdStringA)
   M_OBJECT_PROPERTY_VARIANT                   (XmlNode, Value,      ST_MVariant_X)
   M_OBJECT_PROPERTY_VARIANT                   (XmlNode, Text,       ST_MVariant_X)
   M_OBJECT_PROPERTY_READONLY_STRING           (XmlNode, Path,       ST_MStdString_X)
   M_OBJECT_PROPERTY_READONLY_OBJECT           (XmlNode, Root)
   M_OBJECT_PROPERTY_READONLY_OBJECT           (XmlNode, DocumentElement)
   M_OBJECT_PROPERTY_READONLY_OBJECT           (XmlNode, Parent)
   M_OBJECT_PROPERTY_READONLY_VARIANT          (XmlNode, AllChildren, ST_MVariant_X)
   M_OBJECT_PROPERTY_READONLY_BOOL_EXACT       (XmlNode, HasChildren)
   M_OBJECT_PROPERTY_READONLY_OBJECT           (XmlNode, FirstChild)
   M_OBJECT_PROPERTY_READONLY_OBJECT           (XmlNode, LastChild)
   M_OBJECT_PROPERTY_READONLY_OBJECT           (XmlNode, NextSibling)
   M_OBJECT_PROPERTY_READONLY_OBJECT           (XmlNode, PreviousSibling)
   M_OBJECT_PROPERTY_VARIANT                   (XmlNode, AllAttributes,     ST_MVariant_X)
   M_OBJECT_PROPERTY_READONLY_STRING_COLLECTION(XmlNode, AllAttributeNames, ST_MStdStringVector_X)
M_START_METHODS(XmlNode)
   M_OBJECT_SERVICE       (XmlNode, IsChildPresent,          ST_bool_X_constMStdStringA)
   M_OBJECT_SERVICE       (XmlNode, GetChild,                ST_MObjectP_X_constMStdStringA)
   M_OBJECT_SERVICE       (XmlNode, GetExistingChild,        ST_MObjectP_X_constMStdStringA)
   M_OBJECT_SERVICE       (XmlNode, IsAttributePresent,      ST_bool_X_constMStdStringA)
   M_OBJECT_SERVICE       (XmlNode, RemoveAllAttributes,     ST_X)
   M_OBJECT_SERVICE       (XmlNode, RemoveAttribute,         ST_bool_X_constMStdStringA)
   M_OBJECT_SERVICE       (XmlNode, RemoveExistingAttribute, ST_X_constMStdStringA)
   M_OBJECT_SERVICE       (XmlNode, GetAttribute,            ST_MVariant_X_constMStdStringA)
   M_OBJECT_SERVICE       (XmlNode, GetAttributeAsInt,       ST_int_X_constMStdStringA)
   M_OBJECT_SERVICE       (XmlNode, GetAttributeAsDouble,    ST_double_X_constMStdStringA)
   M_OBJECT_SERVICE       (XmlNode, SetAttribute,            ST_bool_X_constMStdStringA_constMVariantA)
   M_OBJECT_SERVICE       (XmlNode, PrependAttribute,        ST_MObjectP_X_constMStdStringA_constMVariantA)
   M_OBJECT_SERVICE       (XmlNode, AppendAttribute,         ST_MObjectP_X_constMStdStringA_constMVariantA)
   M_OBJECT_SERVICE       (XmlNode, AppendChild,             ST_MObjectP_X_int)
   M_OBJECT_SERVICE       (XmlNode, PrependChild,            ST_MObjectP_X_int)
   M_OBJECT_SERVICE       (XmlNode, AppendChildElement,      ST_MObjectP_X_constMStdStringA)
   M_OBJECT_SERVICE       (XmlNode, PrependChildElement,     ST_MObjectP_X_constMStdStringA)
   M_OBJECT_SERVICE       (XmlNode, RemoveAllChildren,       ST_X)
   M_OBJECT_SERVICE       (XmlNode, RemoveChild,             ST_bool_X_constMVariantA)
   M_OBJECT_SERVICE       (XmlNode, RemoveExistingChild,     ST_X_constMVariantA)
   M_OBJECT_SERVICE       (XmlNode, AppendFragment,          ST_X_constMStdStringA)
   M_OBJECT_SERVICE       (XmlNode, GetFirstElementByPath,   ST_MObjectP_X_constMStdStringA)
M_END_CLASS(XmlNode, Object)

MXmlNode::NodeTypeEnum MXmlNode::GetNodeType() const
{
   return static_cast<MXmlNode::NodeTypeEnum>(DoAccessPugiNode().type());
}

MStdString MXmlNode::GetName() const
{
   return DoAccessPugiNode().name();
}

void MXmlNode::SetName(const MStdString& name)
{
   if ( !DoAccessPugiNode().set_name(name.c_str()) )
   {
      MException::Throw(MException::ErrorSoftware, M_CODE_STR(M_ERR_CANNOT_SET_TO_SUCH_NODE, "Cannot set name to node of this type"));
      M_ENSURED_ASSERT(0);
   }
}

MVariant MXmlNode::GetValue() const
{
   return GetStringValue();
}

void MXmlNode::SetValue(const MVariant& v)
{
   SetStringValue(v.AsString());
}

MStdString MXmlNode::GetStringValue() const
{
   return DoAccessPugiNode().value();
}

void MXmlNode::SetStringValue(const MStdString& v)
{
   if ( !DoAccessPugiNode().set_value(v.c_str()) )
   {
      MException::Throw(MException::ErrorSoftware, M_CODE_STR(M_ERR_CANNOT_SET_TO_SUCH_NODE, "Cannot set value to node of this type"));
      M_ENSURED_ASSERT(0);
   }
}

   class MXmlLocalStreamWriter : public pugi::xml_writer
   {
      MStream* m_stream;

   public:

      MXmlLocalStreamWriter(MStream* stream)
         :
            m_stream(stream)
      {
      }

      virtual ~MXmlLocalStreamWriter()
      {
      }

      virtual void write(const void* data, size_t size)
      {
         m_stream->WriteBytes(reinterpret_cast<const char*>(data), static_cast<unsigned>(size));
      }
   };

MStdString MXmlNode::AsString() const
{
   MStreamMemory mem;
   MXmlLocalStreamWriter writer(&mem);

   const MXmlDocument* doc = GetRootConst();
   if ( doc == this )
      doc->m_document.save(writer, doc->GetIndentationSequence().c_str(), doc->GetFormatMask());
   else
      DoAccessPugiNode().print(writer, doc->GetIndentationSequence().c_str(), doc->GetFormatMask());
   return mem.GetBuffer();
}

MXmlNode* MXmlNode::GetParent() const
{
   return DoAccessXmlNode(DoAccessPugiNode().parent());
}

bool MXmlNode::HasChildren() const
{
   return !!DoAccessPugiNode().first_child();
}

MXmlNode::NodeVector MXmlNode::GetChildren() const
{
   MXmlNode::NodeVector result;
   pugi::xml_node node = DoAccessPugiNode();
   pugi::xml_node::iterator it = node.begin();
   pugi::xml_node::iterator itEnd = node.end();
   for ( ; it != itEnd; ++it )
      result.push_back(DoAccessXmlNode(*it));
   return result;
}

MVariant MXmlNode::GetAllChildren() const
{
   MVariant result;
   result.SetToNull(MVariant::VAR_VARIANT_COLLECTION);
   pugi::xml_node node = DoAccessPugiNode();
   pugi::xml_node::iterator it = node.begin();
   pugi::xml_node::iterator itEnd = node.end();
   for ( ; it != itEnd; ++it )
      result.AddToVariantCollection(DoAccessXmlNode(*it));
   return result;
}

MXmlNode* MXmlNode::GetFirstChild() const
{
   return DoAccessXmlNode(DoAccessPugiNode().first_child());
}

MXmlNode* MXmlNode::GetLastChild() const
{
   return DoAccessXmlNode(DoAccessPugiNode().last_child());
}

MXmlNode* MXmlNode::GetNextSibling() const
{
   return DoAccessXmlNode(DoAccessPugiNode().next_sibling());
}

MXmlNode* MXmlNode::GetPreviousSibling() const
{
   return DoAccessXmlNode(DoAccessPugiNode().previous_sibling());
}

MXmlNode* MXmlNode::GetChild(const MStdString& name) const
{
   return DoAccessXmlNode(DoAccessPugiNode().child(name.c_str()));
}

MXmlNode* MXmlNode::GetExistingChild(const MStdString& name) const
{
   MXmlNode* node = GetChild(name);
   if ( node == NULL )
   {
      MException::ThrowUnknownItem(name);
   }
   return node;
}

bool MXmlNode::IsAttributePresent(const MStdString& name) const
{
   return !DoAccessPugiNode().attribute(name.c_str()).empty();
}

MVariant MXmlNode::GetAllAttributes() const
{
   MVariant result;
   result.SetToNull(MVariant::VAR_MAP);
   pugi::xml_node node = DoAccessPugiNode();
   pugi::xml_attribute_iterator it = node.attributes_begin();
   pugi::xml_attribute_iterator itEnd = node.attributes_end();
   for ( ; it != itEnd; ++it )
   {
      pugi::xml_attribute attr = *it;
      result.SetItem(attr.name(), attr.value());
   }
   return result;
}

void MXmlNode::SetAllAttributes(const MVariant& attrs)
{
   RemoveAllAttributes();
   pugi::xml_node node = DoAccessPugiNode();
   int count = attrs.GetCount(); // this does a necessary check for indexed elements
   for ( int index = 0; index < count; ++index )
   {
      const MStdString& key = attrs.GetMapKeyByIndex(index).AsString();
      const MStdString& value = attrs.GetMapValueByIndex(index).AsString();
      pugi::xml_attribute a = node.append_attribute(key.c_str());
      a.set_value(value.c_str());
   }
}

MStdStringVector MXmlNode::GetAllAttributeNames() const
{
   MStdStringVector result;
   pugi::xml_node node = DoAccessPugiNode();
   pugi::xml_attribute_iterator it = node.attributes_begin();
   pugi::xml_attribute_iterator itEnd = node.attributes_end();
   for ( ; it != itEnd; ++it )
      result.push_back((*it).name());
   return result;
}

void MXmlNode::RemoveAllAttributes()
{
   pugi::xml_node node = DoAccessPugiNode();
   for ( ;; )
   {
      pugi::xml_attribute_iterator it = node.attributes_begin();
      if ( it == node.attributes_end() ) // done removing
         break;
      node.remove_attribute(*it);
   }
}

bool MXmlNode::RemoveAttribute(const MStdString& name)
{
   return DoAccessPugiNode().remove_attribute(name.c_str());
}

void MXmlNode::RemoveExistingAttribute(const MStdString& name)
{
   if ( !RemoveAttribute(name) )
   {
      MException::ThrowUnknownItem(name);
      M_ENSURED_ASSERT(0);
   }
}

MVariant MXmlNode::GetAttribute(const MStdString& name) const
{
   return GetAttributeAsChars(name);
}

MStdString MXmlNode::GetAttributeAsString(const MStdString& name) const
{
   return GetAttributeAsChars(name);
}

MConstChars MXmlNode::GetAttributeAsChars(const MStdString& name) const
{
   pugi::xml_attribute attr = DoAccessPugiNode().attribute(name.c_str());
   if ( attr.empty() )
   {
      MException::ThrowUnknownItem(name);
      M_ENSURED_ASSERT(0);
   }
   return attr.value();
}

int MXmlNode::GetAttributeAsInt(const MStdString& name) const
{
   MConstChars val = GetAttributeAsChars(name);
   return MToInt(val);
}

double MXmlNode::GetAttributeAsDouble(const MStdString& name) const
{
   MConstChars val = GetAttributeAsChars(name);
   return MToDouble(val);
}

bool MXmlNode::SetAttribute(const MStdString& name, const MVariant& value)
{
   pugi::xml_attribute attr = DoAccessPugiNode().attribute(name.c_str());
   if ( attr.empty() )
   {
      AppendAttribute(name, value);
      return true;
   }
   attr.set_value(value.AsString().c_str());
   return false;
}

MXmlNode* MXmlNode::PrependAttribute(const MStdString& name, const MVariant& value)
{
   pugi::xml_attribute attr = DoAccessPugiNode().prepend_attribute(name.c_str());
   DoCheckAttributeAdded(attr);
   attr.set_value(value.AsString().c_str());
   return this;
}

MXmlNode* MXmlNode::AppendAttribute(const MStdString& name, const MVariant& value)
{
   pugi::xml_attribute attr = DoAccessPugiNode().append_attribute(name.c_str());
   DoCheckAttributeAdded(attr);
   attr.set_value(value.AsString().c_str());
   return this;
}

void MXmlNode::InsertAttributeBefore(const MStdString& targetName, const MStdString& name, const MVariant& value)
{
   pugi::xml_node node = DoAccessPugiNode();
   pugi::xml_attribute targetAttr = node.attribute(targetName.c_str());
   if ( targetAttr.empty() )
   {
      MException::ThrowUnknownItem(name);
      M_ENSURED_ASSERT(0);
   }
   pugi::xml_attribute attr = node.insert_attribute_before(name.c_str(), targetAttr);
   DoCheckAttributeAdded(attr);
   attr.set_value(value.AsString().c_str());
}

MXmlNode* MXmlNode::AppendChild(NodeTypeEnum type)
{
   pugi::xml_node result = DoAccessPugiNode().append_child(static_cast<pugi::xml_node_type>(type));
   return DoAccessXmlNodeAfterAdd(result);
}

MXmlNode* MXmlNode::PrependChild(NodeTypeEnum type)
{
   pugi::xml_node result = DoAccessPugiNode().prepend_child(static_cast<pugi::xml_node_type>(type));
   return DoAccessXmlNodeAfterAdd(result);
}

MXmlNode* MXmlNode::InsertChildBefore(const MXmlNode* node, NodeTypeEnum type)
{
   pugi::xml_node result = DoAccessPugiNode().insert_child_before(static_cast<pugi::xml_node_type>(type), node->DoAccessPugiNode());
   return DoAccessXmlNodeAfterAdd(result);
}

MXmlNode* MXmlNode::AppendChildElement(const MStdString& name)
{
   pugi::xml_node result = DoAccessPugiNode().append_child(name.c_str());
   return DoAccessXmlNodeAfterAdd(result);
}

MXmlNode* MXmlNode::PrependChildElement(const MStdString& name)
{
   pugi::xml_node result = DoAccessPugiNode().prepend_child(name.c_str());
   return DoAccessXmlNodeAfterAdd(result);
}

MXmlNode* MXmlNode::InsertChildElementBefore(const MXmlNode* node, const MStdString& name)
{
   pugi::xml_node result = DoAccessPugiNode().insert_child_before(name.c_str(), node->DoAccessPugiNode());
   return DoAccessXmlNodeAfterAdd(result);
}

void MXmlNode::AppendFragment(const MStdString& contents)
{
   AppendFragmentFromBuffer(contents.data(), static_cast<unsigned>(contents.size()));
}

void MXmlNode::AppendFragmentFromBuffer(const char* buff, unsigned size)
{
   MXmlDocument* doc = GetRoot();
   pugi::xml_parse_result result = DoAccessPugiNode().append_buffer(buff, size, doc->GetParseMask());
   doc->DoHandleParseResult(result, buff);
}

void MXmlNode::RemoveAllChildren()
{
   pugi::xml_node node = DoAccessPugiNode();
   for ( ;; )
   {
      pugi::xml_node::iterator it = node.begin();
      if ( it == node.end() ) // done removing
         break;
      node.remove_child(*it);
   }
}

bool MXmlNode::RemoveChild(const MVariant& nameOrNodeObject)
{
   if ( nameOrNodeObject.IsObject() )
   {
      MXmlNode* node = M_DYNAMIC_CAST_WITH_THROW(MXmlNode, nameOrNodeObject.AsExistingObject());
      return RemoveChildByObject(node);
   }
   return RemoveChildByName(nameOrNodeObject.AsString());
}

bool MXmlNode::RemoveChildByName(const MStdString& name)
{
   return DoAccessPugiNode().remove_child(name.c_str());
}

bool MXmlNode::RemoveChildByObject(MXmlNode* node)
{
   if ( node == NULL )
      return false;
   return DoAccessPugiNode().remove_child(node->DoAccessPugiNode());
}

void MXmlNode::RemoveExistingChild(const MVariant& nameOrNodeObject)
{
   if ( nameOrNodeObject.IsObject() )
   {
      MXmlNode* node = M_DYNAMIC_CAST_WITH_THROW(MXmlNode, nameOrNodeObject.AsExistingObject());
      RemoveExistingChildByObject(node);
   }
   else
      RemoveExistingChildByName(nameOrNodeObject.AsString());
}

void MXmlNode::RemoveExistingChildByName(const MStdString& name)
{
   if ( !RemoveChildByName(name) )
   {
      MException::ThrowUnknownItem(name);
      M_ENSURED_ASSERT(0);
   }
}

void MXmlNode::RemoveExistingChildByObject(MXmlNode* node)
{
   if ( !RemoveChildByObject(node) )
   {
      if ( node == NULL )
         MException::ThrowUnknownItem("");
      else
         MException::ThrowUnknownItem(node->GetName());
      M_ENSURED_ASSERT(0);
   }
}

MVariant MXmlNode::GetText() const
{
   return GetStringText();
}

void MXmlNode::SetText(const MVariant& v)
{
   SetStringText(v.AsString());
}

   static pugi::xml_node DoGetOnlyChildWithValue(pugi::xml_node node)
   {
      for ( pugi::xml_node i = node.first_child(); i; i = i.next_sibling() )
         if ( pugi::impl::is_text_node(i.internal_object()) )
            return i;
      return pugi::xml_node();
   }

MStdString MXmlNode::GetStringText() const
{
   MStdString result;
   pugi::xml_node node = DoGetOnlyChildWithValue(DoAccessPugiNode());
   if ( node )
      result = node.value();
   return result;
}

void MXmlNode::SetStringText(const MStdString& v)
{
   MStdString result;
   pugi::xml_node node = DoGetOnlyChildWithValue(DoAccessPugiNode());
   if ( node )
   {
      bool result = node.set_value(v.c_str());
      M_USED_VARIABLE(result);
      M_ASSERT(result);
   }
   else
      PrependChild(NodePcdata)->SetStringValue(v);
}

   static void DoRecurseForPath(MStdString& result, pugi::xml_node node, char delimiter)
   {
      if ( node.parent() )
      {
         DoRecurseForPath(result, node.parent(), delimiter);
         result += delimiter;
      }
      result += node.name();
   }

MStdString MXmlNode::GetPath() const
{
   MStdString result;
   const MXmlDocument* doc = GetRootConst();
   DoRecurseForPath(result, DoAccessPugiNode(), doc->GetPathDelimiter());
   return result;
}

MXmlNode* MXmlNode::GetFirstElementByPath(const MStdString& path) const
{
   const MXmlDocument* doc = GetRootConst();
   pugi::xml_node result = DoAccessPugiNode().first_element_by_path(path.c_str(), doc->GetPathDelimiter());
   if ( !result )
   {
      MException::ThrowUnknownItem(path);
      M_ENSURED_ASSERT(0);
   }
   return DoAccessXmlNode(result);
}

MXmlDocument* MXmlNode::GetRoot()
{
   pugi::xml_node result = DoAccessPugiNode().root();
   return M_CHECKED_CAST(MXmlDocument*, DoAccessXmlNode(result));
}

MXmlNode* MXmlNode::GetDocumentElement() const
{
   pugi::xml_node doc = DoAccessPugiNode().root();
   pugi::xml_node node = doc.last_child(); // better to start from the end, hit the element faster
   for ( ; node; node = node.previous_sibling() )
      if (node.type() == pugi::node_element )
         return DoAccessXmlNode(node);
   return NULL;
}

MXmlNode* MXmlNode::DoAccessXmlNode(pugi::xml_node node) const
{
   pugi::xml_node_struct* s = node.internal_object();
   if ( s != NULL )
   {
      if ( node.type() == pugi::node_document ) // return main document node for top level document
      {
         pugi::impl::xml_document_struct* docStruct = M_CHECKED_CAST(pugi::impl::xml_document_struct*, s);
         return docStruct->m_document;
      }
      return node.internal_object();
   }
   return NULL;
}

MXmlNode* MXmlNode::DoAccessXmlNodeAfterAdd(pugi::xml_node node) const
{
   MXmlNode* x = DoAccessXmlNode(node);
   if ( x == NULL )
   {
      MException::Throw(MException::ErrorSoftware, M_CODE_STR(M_ERR_SYNTAX_ERROR_IN_S1, "Cannot add child to node of such type"));
      M_ENSURED_ASSERT(0);
   }
   return x;
}

void MXmlNode::DoCheckAttributeAdded(pugi::xml_attribute attr)
{
   if ( !attr )
   {
      MException::Throw(MException::ErrorSoftware, M_CODE_STR(M_ERR_SYNTAX_ERROR_IN_S1, "Cannot add attribute node of such type"));
      M_ENSURED_ASSERT(0);
   }
}

pugi::xml_node MXmlNode::DoAccessPugiNode() const
{
   return pugi::xml_node((pugi::xml_node_struct*)this);
}

   #if !M_NO_REFLECTION

     /// Create an empty XML document, ready to be read or populated manually.
     ///
     /// No child nodes are present.
     ///
     static MXmlDocument* DoNew0()
     {
        return M_NEW MXmlDocument();
     }

     /// Create an XML document from generic parameter.
     ///
     /// An error can result from stream I/O, or if the document is malformed.
     ///
     /// \param streamFilenameOrString
     ///     The parameter can be one of the following types:
     ///       - Instance of class MXmlDocument. If provided, this document will be a copy.
     ///       - MStream object that is opened and ready to be read.
     ///         The whole stream will be read, but there will be no attempt to close the stream.
     ///       - An in-place XML document in a possibly long string.
     ///         Whether this is an in-place XML is determined by having '<' at the beginning
     ///         and '>' at the end of the string.
     ///       - A file name.
     ///
     static MXmlDocument* DoNew1(const MVariant& streamFilenameOrString)
     {
        return M_NEW MXmlDocument(streamFilenameOrString);
     }

     /// Create an XML document from generic parameter and a parse mask.
     ///
     /// An error can result from stream I/O, or in case the document is malformed.
     ///
     /// \param streamFilenameOrString
     ///     The parameter can be one of the following types:
     ///       - Instance of class MXmlDocument. If provided, this document will be a copy.
     ///       - MStream object that is opened and ready to be read.
     ///         The whole stream will be read, but there will be no attempt to close the stream.
     ///       - An in-place XML document in a possibly long string.
     ///         Whether this is an in-place XML is determined by having '<' at the beginning
     ///         and '>' at the end of the string.
     ///       - A file name.
     /// \param parseMask
     ///      Parse mask to use. If not given, use ParseMaskDefault.
     ///
     static MXmlDocument* DoNew2(const MVariant& streamFilenameOrString, unsigned parseMask)
     {
        return M_NEW MXmlDocument(streamFilenameOrString, parseMask);
     }

   #endif

M_START_PROPERTIES(XmlDocument)
   M_CLASS_ENUMERATION               (XmlDocument, ParsePi)
   M_CLASS_ENUMERATION               (XmlDocument, ParseComments)
   M_CLASS_ENUMERATION               (XmlDocument, ParseCdata)
   M_CLASS_ENUMERATION               (XmlDocument, ParseWsPcdata)
   M_CLASS_ENUMERATION               (XmlDocument, ParseEscapes)
   M_CLASS_ENUMERATION               (XmlDocument, ParseEol)
   M_CLASS_ENUMERATION               (XmlDocument, ParseWconvAttribute)
   M_CLASS_ENUMERATION               (XmlDocument, ParseWnormAttribute)
   M_CLASS_ENUMERATION               (XmlDocument, ParseDeclaration)
   M_CLASS_ENUMERATION               (XmlDocument, ParseDoctype)
   M_CLASS_ENUMERATION               (XmlDocument, ParseWsPcdataSingle)
   M_CLASS_ENUMERATION               (XmlDocument, ParseTrimPcdata)
   M_CLASS_ENUMERATION               (XmlDocument, ParseFragment)
   M_CLASS_ENUMERATION               (XmlDocument, ParseMaskMinimal)
   M_CLASS_ENUMERATION               (XmlDocument, ParseMaskDefault)
   M_CLASS_ENUMERATION               (XmlDocument, ParseMaskFull)
   M_CLASS_ENUMERATION               (XmlDocument, FormatIndent)
   M_CLASS_ENUMERATION               (XmlDocument, FormatWriteBom)
   M_CLASS_ENUMERATION               (XmlDocument, FormatRaw)
   M_CLASS_ENUMERATION               (XmlDocument, FormatNoDeclaration)
   M_CLASS_ENUMERATION               (XmlDocument, FormatNoEscapes)
   M_CLASS_ENUMERATION               (XmlDocument, FormatSaveFileText)
   M_CLASS_ENUMERATION               (XmlDocument, FormatIndentAttributes)
   M_CLASS_ENUMERATION               (XmlDocument, FormatMaskDefault)
   M_OBJECT_PROPERTY_UINT            (XmlDocument, ParseMask)
   M_OBJECT_PROPERTY_UINT            (XmlDocument, FormatMask)
   M_OBJECT_PROPERTY_STRING          (XmlDocument, IndentationSequence, ST_constMStdStringA_X, ST_X_constMStdStringA)
   M_OBJECT_PROPERTY_CHAR            (XmlDocument, PathDelimiter)
M_START_METHODS(XmlDocument)
   M_OBJECT_SERVICE                  (XmlDocument, Clear,           ST_X)
   M_OBJECT_SERVICE                  (XmlDocument, Read,            ST_X_constMVariantA)
   M_OBJECT_SERVICE                  (XmlDocument, Write,           ST_X_constMVariantA)
   M_OBJECT_SERVICE                  (XmlDocument, Assign,          ST_X_MObjectP)
   M_CLASS_FRIEND_SERVICE_OVERLOADED (XmlDocument, New, DoNew0, 0,  ST_MObjectP_S)
   M_CLASS_FRIEND_SERVICE_OVERLOADED (XmlDocument, New, DoNew1, 1,  ST_MObjectP_S_constMVariantA)
   M_CLASS_FRIEND_SERVICE_OVERLOADED (XmlDocument, New, DoNew2, 2,  ST_MObjectP_S_constMVariantA_unsigned)
M_END_CLASS(XmlDocument, XmlNode)

MXmlDocument::MXmlDocument()
:
   MXmlNode(),
   m_parseMask(ParseMaskDefault),
   m_formatMask(FormatMaskDefault),
   m_indentationSequence("   ", 3),
   m_pathDelimiter('/')
{
   Clear();
}

MXmlDocument::MXmlDocument(const MVariant& streamFilenameOrString, unsigned parseMask)
:
   MXmlNode(),
   m_parseMask(parseMask),
   m_formatMask(FormatMaskDefault),
   m_indentationSequence("   ", 3),
   m_pathDelimiter('/')
{
   Read(streamFilenameOrString);
}

MXmlDocument::MXmlDocument(const void* buffer, unsigned size, unsigned parseMask)
:
   MXmlNode(),
   m_parseMask(parseMask),
   m_formatMask(FormatMaskDefault),
   m_indentationSequence("   ", 3),
   m_pathDelimiter('/')
{
   ReadFromBuffer(buffer, size);
}

MXmlDocument::~MXmlDocument()
{
}

void MXmlDocument::Read(const MVariant& streamFilenameOrString)
{
   if ( streamFilenameOrString.GetType() == MVariant::VAR_OBJECT )
   {
      MObject* obj = streamFilenameOrString.AsExistingObject();
      MStream* stream = M_DYNAMIC_CAST(MStream, obj);
      if ( stream != NULL )
         ReadFromStream(stream);
      else
      {
         MXmlDocument* xml = M_DYNAMIC_CAST_WITH_THROW(MXmlDocument, obj);
         Assign(*xml);
      }
   }
   else
   {
      const MStdString& str = streamFilenameOrString.AsString();
      if ( str.size() > M_MAX_PATH )
         ReadFromString(str);
      else
      {
         pugi::xml_encoding encoding = pugi::impl::get_buffer_encoding(pugi::encoding_auto, str.data(), str.size());
         if ( (encoding != pugi::encoding_utf8 && encoding != pugi::encoding_latin1) )
            ReadFromString(str);
         else
         {
            // efficiently skip blanks. Minimalist XML file is <a/> (and we cannot open a file with such name using this generalized Read call)
            MStdString::const_iterator it = str.begin();
            MStdString::const_iterator itEnd = str.end();
            for ( ; ; ++it )
            {
               if ( it == itEnd )
               {
                  ReadFromString(str); // empty string, empty doc
                  return;
               }
               char c = *it;
               if ( c != '\xEF' && c != '\xBB' && c != '\xBF' && !isspace(*it) ) // skip possible utf-8 BOM (Byte Order Mark) sequence "EF BB BF"
                  break;
            }
            for ( ;; ) // we know for sure the string does not consist of only spaces
            {
               --itEnd;
               if ( !isspace(*itEnd) )
                  break;
            }
            if ( *it == '<' && *itEnd == '>' )
               ReadFromString(str);
            else
               ReadFromFile(str);
         }
      }
   }
}

void MXmlDocument::ReadFromString(const MStdString& xmlString)
{
   ReadFromBuffer(xmlString.data(), static_cast<unsigned>(xmlString.size()));
}

void MXmlDocument::ReadFromStream(MStream* stream)
{
   Clear();
   m_fileName = stream->GetName();
   ReadFromString(stream->ReadAll());
}

void MXmlDocument::ReadFromFile(const MStdString& fileName)
{
   Clear();
   m_fileName = fileName; // this can be overwritten later within ReadFromStream
   MStreamFile file(fileName, MStreamFile::FlagReadOnly, MStreamFile::SharingAllowRead);
   ReadFromStream(&file);
}

void MXmlDocument::ReadFromBuffer(const void* buffer, unsigned size)
{
   Clear();
   pugi::xml_parse_result result = m_document.load_buffer(buffer, size, m_parseMask);
   DoHandleParseResult(result, reinterpret_cast<const char*>(buffer));
   pugi::impl::xml_document_struct* docStruct = M_CHECKED_CAST(pugi::impl::xml_document_struct*, m_document.internal_object());
   docStruct->m_document = this;
}

void MXmlDocument::DoHandleParseResult(pugi::xml_parse_result& result, const char* text)
{
   if ( result.status != pugi::status_ok )
   {
      MException ex(result.description(), M_ERR_SYNTAX_ERROR_IN_S1);
      pugi::xml_encoding encoding = pugi::impl::get_buffer_encoding(pugi::encoding_auto, text, result.offset);
      if ( encoding == pugi::encoding_utf8 || encoding == pugi::encoding_latin1 )
      {
         unsigned line = 1;
         const char* it = text;
         const char* itEnd = it + result.offset;
         for ( ; it != itEnd && *it; ++it )
            if ( *it == '\n' )
               ++line;

#if !M_NO_VERBOSE_ERROR_INFORMATION
         ex.SetFileNameAndLineNumber(MFileNameAndLineNumber(m_fileName, line));
#endif

         // find a chunk of offending code
         for ( int i = 32; i > 0 && it >= text; --i, --it )
            ;
         size_t len = itEnd - it;
         if ( len > 0 )
            ex.AppendToString(" after '%s'", MStr::ToEscapedString(MStdString(it, len)).c_str());
      }
      ex.Rethrow();
      M_ENSURED_ASSERT(0);
   }
}

void MXmlDocument::Clear()
{
   m_fileName.clear();
   m_document.reset();
   pugi::impl::xml_document_struct* docStruct = M_CHECKED_CAST(pugi::impl::xml_document_struct*, m_document.internal_object());
   docStruct->m_document = this;
}

void MXmlDocument::Write(const MVariant& streamOrFilename)
{
   if ( streamOrFilename.GetType() == MVariant::VAR_OBJECT )
   {
      MStream* stream = M_DYNAMIC_CAST_WITH_THROW(MStream, streamOrFilename.AsExistingObject());
      WriteToStream(stream);
   }
   else
      WriteToFile(streamOrFilename.AsString());
}

void MXmlDocument::WriteToStream(MStream* stream)
{
   MXmlLocalStreamWriter writer(stream);
   m_document.print(writer, m_indentationSequence.c_str(), m_formatMask);
}

void MXmlDocument::WriteToFile(const MStdString& fileName)
{
   MStreamFile file(fileName, MStreamFile::FlagWriteOnly | MStreamFile::FlagCreate | MStreamFile::FlagTruncate);
   WriteToStream(&file);
}

void MXmlDocument::Assign(const MXmlDocument& other)
{
   M_DYNAMIC_CAST_WITH_THROW(MXmlDocument, &other); // check for the case of reflection call
   m_document.reset(other.m_document);
}

MXmlDocument* MXmlDocument::GetRoot()
{
   return this;
}

pugi::xml_node MXmlDocument::DoAccessPugiNode() const
{
   return m_document;
}

#endif // !M_NO_XML
