#ifndef MCORE_MXMLDOCUMENT_H
#define MCORE_MXMLDOCUMENT_H
/// \file MCORE/MXmlDocument.h
/// \addtogroup MCORE
///@{

#include <MCORE/MCOREDefs.h>
#include <MCORE/MObject.h>

#if !M_NO_XML

#include <MCORE/private/pugiconfig.hpp>
#include <MCORE/private/pugixml.hpp>

/// DOM representation of XML node.
///
/// All different kinds of nodes are represented by this single class.
/// Possible node types are defined by \refprop{GetNodeType,NodeType}.
///
class M_CLASS MXmlNode : public MObject
{
   friend class M_CLASS MXmlDocument;

public: // Types:

   /// Type of the node
   ///
   enum NodeTypeEnum
   {
      /// Document tree root node.
      ///
      /// Document has neither Name, nor Value, nor does it have Text.
      /// Typically, document has xml processing instruction of node type NodePi,
      /// comments (of type NodeComment), and the main single node element
      /// accessible from any node with property DocumentElement.
      ///
      NodeDocument = 1,

      /// Element, the most common node type.
      ///
      /// Elements have nonempty property Name, but there is no Value property.
      /// Elements can have children and property Text, which is a child of type NodePcdata.
      /// Here is the placement of all properties of the element:
      ///
      /// \code
      ///    <Name attr1="attr-value1" attr2="attr-value2">Text</Name>
      /// \endcode
      ///
      NodeElement = 2,

      /// Plain character data such as 'plain characters'.
      ///
      /// This node has only value. It cannot have children, attributes or name.
      ///
      NodePcdata = 3,

      /// Character data block such as '<![CDATA[characters]]>'.
      ///
      /// This node has only value. It cannot have children, attributes or name.
      ///
      NodeCdata = 4,

      /// XML comment such as '<!-- comment is here -->'.
      ///
      /// This node has only value. It cannot have children, attributes or name.
      ///
      NodeComment = 5,

      /// Processing instruction such as '<?processing ?>'.
      ///
      /// This node has name and value. It cannot have children or attributes.
      /// Here is the placement of all properties of the element:
      ///
      /// \code
      ///    <?Name Value?>
      /// \endcode
      ///
      NodePi = 6,

      /// Document declaration such as '<?xml version="1.0"?>'.
      ///
      /// Document declaration is typically the first node of the document.
      /// While it has similar syntax to NodePi, this node can have name and attributes,
      /// but cannot have children and Value.
      ///
      /// \code
      ///    <?Name attr1="attr-value1" attr2="attr-value2"?>
      /// \endcode
      ///
      /// Name is typically "xml".
      ///
      NodeDeclaration = 7,

      /// Document type declaration, such as '<!DOCTYPE doc>'.
      ///
      /// This type has name and value, but not children or attributes.
      /// Placement of properties:
      ///
      /// \code
      ///    <!Name Value!>
      /// \endcode
      ///
      NodeDoctype = 8
   };

   /// Convenience type, vector of nodes.
   ///
   typedef M_TEMPLATE_CLASS std::vector<MXmlNode*>
        NodeVector;

public: // Types:

   /// Type of the node.
   ///
   /// All different XML nodes have the same class, but they differentiate by type.
   /// Possible values for this property are defined in the enumeration:
   ///  - NodeDocument = 1
   ///  - NodeElement = 2
   ///  - NodePcdata = 3
   ///  - NodeCdata = 4
   ///  - NodeComment = 5
   ///  - NodePi = 6
   ///  - NodeDeclaration = 7
   ///  - NodeDoctype = 8
   ///
   NodeTypeEnum GetNodeType() const;

   ///@{
   /// Name of the node.
   ///
   /// Only NodeElement, NodePi, NodeDeclaration, and NodeDoctype will have nonempty name.
   /// When assigning Name to nodes of other type, an exception will be thrown.
   /// When getting Name to nodes of other type, empty string is returned.
   ///
   /// Here are the examples of names in different node types:
   ///
   /// \code
   ///    <Name attr1="attr-val1">    <!-- NodeElement -->
   ///    <!Name Value!>              <!-- NodeDoctype -->
   ///    <?Name attr1="attr-val1"?>  <!-- NodeDeclaration -->
   ///    <?Name Value!>              <!-- NodePi -->
   /// \endcode
   ///
   MStdString GetName() const;
   void SetName(const MStdString&);
   ///@}

   ///@{
   /// Value of the node.
   ///
   /// When parsed from a document, values will be present in element types
   /// NodePcdata, NodeCdata, NodeComment, NodePi, and in NodeDoctype. Here are examples:
   ///
   /// \code
   ///    <![CDATA[this is the value]]>
   ///    this is also the value, pcdata
   ///    <!-- in a comment, this text is a value -->
   ///    <!DOCTYPE this is a value of doctype>
   /// \endcode
   ///
   /// All other node types will return empty strings when accessed,
   /// and will throw an exception when assigned.
   ///
   MVariant GetValue() const;
   void SetValue(const MVariant&);
   ///@}

   ///@{
   /// String representation of value, C++ convenience function.
   ///
   /// \seeprop{GetValue, Value} - same as this, but handles a variant data type.
   ///
   MStdString GetStringValue() const;
   void SetStringValue(const MStdString&);
   ///@}

   ///@{
   /// Text associated with the node.
   ///
   /// Accessing Text of any node of type other than NodeElement will always return an empty string.
   /// Assigning Text to a non-NodeElement will throw an exception as only NodeElement can have children.
   ///
   /// Texts inside node elements are either of type NodeCdata or NodePcdata.
   /// Text property makes it convenient to access such immediate child from parent.
   /// Here is where the text is located:
   ///
   /// \code
   ///    <records>
   ///     <name>text value is here, manageable by Text property</name>
   ///    </records>
   /// \endcode
   ///
   /// Therefore, having node record:
   ///
   /// \if CPP
   ///   \code
   ///      record->GetChild("name")->GetText()
   ///   \endcode
   /// \endif
   ///
   /// If the node does not have a cnode or pcnode, an empty string is returned.
   /// When the property is assigned but there is no cnode child, a cnode is created and value is assigned to it.
   ///
   MVariant GetText() const;
   void SetText(const MVariant&);
   ///@}

   ///@{
   /// String representation of text, C++ convenience function.
   ///
   /// \seeprop{GetText, Text} - same as this, but handles a variant data type.
   ///
   MStdString GetStringText() const;
   void SetStringText(const MStdString&);
   ///@}

   /// XML String representation of this element, and all its children.
   ///
   /// The result would look like:
   ///
   /// \code
   ///    <records><name>text value is here, manageable by Text property</name></records>
   /// \endcode
   ///
   /// The exact format - whether there are new lines and how indentation is performed -
   /// is sensitive to \refprop{MXmlDocument.GetParseMask, XmlDocument.ParseMask} and
   /// \refprop{MXmlDocument.GetIndentationSequence, XmlDocument.IndentationSequence}.
   ///
   MStdString AsString() const;

   /// Access parent of this node.
   ///
   /// The only node that has no parent is document node, of type NodeDoctype.
   /// Such node returns a null pointer as its parent.
   ///
   MXmlNode* GetParent() const;

   /// Whether the node has one or more children.
   ///
   /// \see RemoveAllChildren - remove all children of the node.
   /// \seeprop{GetAllChildren,AllChildren} - return an array of all children.
   ///
   bool HasChildren() const;

   /// Get the read-only array of all children of the node.
   ///
   /// The returned variant array has all children of the node.
   ///
   /// \see GetAllChildren - reflected version of this call that returns a variant.
   ///
   MXmlNode::NodeVector GetChildren() const;

   /// Get the read-only array of all children of the node.
   ///
   /// The returned variant array has all children of the node.
   ///
   MVariant GetAllChildren() const;

   /// Return the first child of the node, if present.
   ///
   /// Children form a circular double linked list, but there is the first and the last child in the ring.
   /// If the node has no children, null is returned.
   ///
   /// \see GetChild - access child by name, or return null if there is no such child.
   /// \see GetExistingChild - access child by name, throws an exception if there is no such child.
   ///
   MXmlNode* GetFirstChild() const;

   /// Return the last child of the node, if present.
   ///
   /// Children form a circular double linked list, but there is the first and the last child in the ring.
   /// If the node has no children, null is returned.
   ///
   /// \see GetChild - access child by name, or return null if there is no such child.
   /// \see GetExistingChild - access child by name, throws an exception if there is no such child.
   ///
   MXmlNode* GetLastChild() const;

   /// Return the sibling that is previous to this node.
   ///
   /// Nodes form a circular double linked list, therefore, when traversing through previous siblings
   /// one reaches the initial node. If the node has no siblings, its previous sibling is self.
   ///
   MXmlNode* GetPreviousSibling() const;

   /// Return the sibling that is next to this node.
   ///
   /// Nodes form a circular double linked list, therefore, when traversing through next siblings
   /// one reaches the initial node. If the node has no siblings, its next sibling is self.
   ///
   MXmlNode* GetNextSibling() const;

   /// Access the first child by name, if it is present.
   ///
   /// If such child is not present, null is returned.
   /// If there is more than one child with such name, one cannot reach them all with this call.
   /// The memory of the created node is managed by the document, the node should not be deleted.
   ///
   /// \param name
   ///    Name of the child.
   ///
   /// \return MXmlNode, child with the given name or null.
   ///
   /// \see GetExistingChild - access child by name, throws an exception if there is no such child.
   /// \seeprop{GetFirstChild, FirstChild} - access first child of this node.
   /// \seeprop{GetLastChild, LastChild} - access last child of this node.
   ///
   MXmlNode* GetChild(const MStdString& name) const;

   /// Is the child with such name present within the node.
   ///
   /// \param name
   ///    Name of the child.
   ///
   /// \return bool, whether such child is present.
   ///
   bool IsChildPresent(const MStdString& name) const
   {
      return GetChild(name) != NULL;
   }

   /// Access the first child by name, or throw an exception if there is no such child.
   ///
   /// If such child is not present, throw an error.
   /// If there is more than one child with such name, one cannot reach them all with this call.
   /// The memory of the created node is managed by the document, the node should not be deleted.
   ///
   /// \param name
   ///    Name of the child.
   ///
   /// \return MXmlNode, child with the given name, never a null.
   ///
   /// \see GetChild - if there is no such node return null.
   /// \seeprop{GetFirstChild, FirstChild} - access first child of this node.
   /// \seeprop{GetLastChild, LastChild} - access first child of this node.
   ///
   MXmlNode* GetExistingChild(const MStdString& name) const;

   /// Remove all attributes of the item.
   ///
   void RemoveAllAttributes();

   /// Whether an attribute with such name is present in the node.
   ///
   /// \param name
   ///    Node of the attribute.
   /// \return bool, whether such attribute is present.
   ///
   bool IsAttributePresent(const MStdString& name) const;

   ///@{
   /// Access map of names and values that comprises attributes of this node.
   ///
   /// When getting the map, no type transformation is performed, strings are returned for values.
   /// When setting values, every value is converted into string.
   ///
   MVariant GetAllAttributes() const;
   void SetAllAttributes(const MVariant&);
   ///@}

   /// Access the collection of node attribute names.
   ///
   /// If the node does not have any attributes an empty collection is returned.
   ///
   MStdStringVector GetAllAttributeNames() const;

   /// Remove attribute by name or do nothing if there is no such attribute already.
   ///
   /// \param name
   ///    Name of the attribute to remove.
   ///
   /// \return bool - True if the attribute existed and it was removed.
   ///
   /// \see RemoveExistingAttribute - will thrown an error if there is no such attribute.
   ///
   bool RemoveAttribute(const MStdString& name);

   /// Remove attribute by name.
   ///
   /// If there was no such attribute, throw an error.
   ///
   /// \param name
   ///    Name of the attribute to remove.
   ///
   /// \see RemoveAttribute - will not throw, but instead return false if there is no such attribute.
   ///
   void RemoveExistingAttribute(const MStdString& name);

   /// Get attribute value by name.
   ///
   /// If the node does not have such attribute throw an exception.
   ///
   /// \param name
   ///    Name of the attribute.
   ///
   /// \return MVariant - value, convertible to a type that the user prefers.
   ///
   MVariant GetAttribute(const MStdString& name) const;

   /// Get string attribute value by name, a C++ convenience call.
   ///
   /// \param name
   ///    Name of the attribute.
   ///
   /// \see GetAttribute that returns MVariant type.
   ///
   MStdString GetAttributeAsString(const MStdString& name) const;

   /// Get const char* attribute value, a C++ convenience call.
   ///
   /// \param name
   ///    Name of the attribute.
   ///
   /// \see GetAttribute that returns MVariant type.
   ///
   MConstChars GetAttributeAsChars(const MStdString& name) const;

   /// Get integer attribute value, a C++ convenience call.
   ///
   /// If the attribute is not convertible to integer, an exception is thrown.
   ///
   /// \param name
   ///    Name of the attribute.
   ///
   /// \see GetAttribute that returns MVariant type.
   ///
   int GetAttributeAsInt(const MStdString& name) const;

   /// Get a double precision floating point attribute value, a C++ convenience call.
   ///
   /// If the attribute is not convertible to a double, an exception is thrown.
   ///
   /// \param name
   ///    Name of the attribute.
   ///
   /// \see GetAttribute that returns MVariant type.
   ///
   double GetAttributeAsDouble(const MStdString& name) const;

   /// Set a value to attribute of a given name.
   ///
   /// If such attribute does not exist, but the node type assumes presence of attributes,
   /// a new attribute is created. Otherwise an existing value is modified.
   ///
   /// \param name
   ///    Name of the attribute to modify.
   /// \param value
   ///    The new value of the attribute.
   /// \return bool, True if the value had to be added, false if an existing attribute modified.
   ///
   /// \see PrependAttribute - prepend attribute with given name and value.
   /// \see AppendAttribute - append attribute with given name and value.
   ///
   bool SetAttribute(const MStdString& name, const MVariant& value);

   /// Create an attribute that will be the first in the list of node attributes.
   ///
   /// No check is done whether the attribute with such name is already present.
   ///
   /// \param name
   ///    Name of the attribute to modify.
   /// \param value
   ///    The new value of the attribute.
   /// \return MXmlNode the call returns this object so the calls can be chained.
   ///
   /// \see AppendAttribute - append attribute with given name and value.
   ///
   MXmlNode* PrependAttribute(const MStdString& name, const MVariant& value);

   /// Create an attribute that will be the last in the list of node attributes.
   ///
   /// No check is done whether the attribute with such name is already present.
   ///
   /// \param name
   ///    Name of the attribute to modify.
   /// \param value
   ///    The new value of the attribute.
   /// \return MXmlNode the call returns this object so the calls can be chained.
   ///
   /// \see PrependAttribute - prepend attribute with given name and value.
   ///
   MXmlNode* AppendAttribute(const MStdString& name, const MVariant& value);

   /// Create an attribute and place it before another attribute.
   ///
   /// No check is done whether the attribute with such name is already present.
   ///
   /// \param targetName
   ///    Name of the attribute before which a new one should be appended.
   /// \param name
   ///    Name of the attribute to modify.
   /// \param value
   ///    The new value of the attribute.
   ///
   /// \see AppendAttribute - append attribute with given name and value.
   /// \see PrependAttribute - prepend attribute with given name and value.
   ///
   void InsertAttributeBefore(const MStdString& targetName, const MStdString& name, const MVariant& value);

   /// Append a child node of a given type and return a freshly created node.
   ///
   /// The memory of the created node is managed by the document, the node should not be deleted.
   ///
   /// \param type
   ///    Type of the node to create.
   /// \return MXmlNode that is appended to this node.
   ///
   /// \see PrependChild
   ///
   MXmlNode* AppendChild(NodeTypeEnum type);

   /// Prepend a child node of a given type and return a freshly created node.
   ///
   /// The memory of the created node is managed by the document, the node should not be deleted.
   ///
   /// \param type
   ///    Type of the node to create.
   /// \return MXmlNode that is prepended to this node.
   ///
   /// \see AppendChild
   ///
   MXmlNode* PrependChild(NodeTypeEnum type);

   /// Insert a child node before the given node.
   ///
   /// The memory of the created node is managed by the document, the node should not be deleted.
   ///
   /// \param node
   ///    Child node before which a new one needs to be prepended.
   /// \param type
   ///    Type of the node to create.
   /// \return MXmlNode that is inserted to this node.
   ///
   MXmlNode* InsertChildBefore(const MXmlNode* node, NodeTypeEnum type);

   /// Append element child node and return a freshly created element object.
   ///
   /// The appended child will be the last in the list if children.
   /// The memory of the created node is managed by the document, the node should not be deleted.
   ///
   /// \param name
   ///    The name of the element that will be created.
   /// \return MXmlNode of type NodeElement that is appended to the list if children.
   ///
   /// \see AppendChild - append a generic child.
   ///
   MXmlNode* AppendChildElement(const MStdString& name);

   /// Prepend element child node and return a freshly created element object.
   ///
   /// The prepended child will be the last in the list of children.
   /// The memory of the created node is managed by the document, the node should not be deleted.
   ///
   /// \param name
   ///    The name of the element that will be created.
   /// \return MXmlNode of type NodeElement that is prepended to the list if children.
   ///
   /// \see PrependChild - append a generic child.
   ///
   MXmlNode* PrependChildElement(const MStdString& name);

   /// Insert element child node prior to a given child.
   ///
   /// The appended child will be the last in the list if children.
   /// The memory of the created node is managed by the document, the node should not be deleted.
   ///
   /// \param node
   ///    The child before which the new element is to be created.
   /// \param name
   ///    The name of the element that will be created.
   /// \return MXmlNode of type NodeElement that is appended to the list if children.
   ///
   /// \see AppendChild - append a generic child.
   ///
   MXmlNode* InsertChildElementBefore(const MXmlNode* node, const MStdString& name);

   /// Parse the string buffer as an XML document fragment and append all nodes as children to the current node.
   ///
   /// Copies/converts the buffer, so it may be deleted or changed after the function returns.
   ///
   /// \param contents
   ///    XML fragment that is to be parsed and added to this node as a set of children.
   ///
   void AppendFragment(const MStdString& contents);

   /// Parse the raw buffer as an XML document fragment and append all nodes as children to the current node.
   ///
   /// \param buff
   ///    Pointer to XML fragment that is to be parsed and added to this node as a set of children.
   /// \param size
   ///    Byte size of the argument buff.
   ///
   /// \see AppendFragment - append a string.
   ///
   void AppendFragmentFromBuffer(const char* buff, unsigned size);

   /// Parse the raw buffer as an XML document fragment and append all nodes as children to the current node.
   ///
   /// \param buff
   ///    Zero terminated string, XML fragment that is to be parsed and added to this node as a set of children.
   ///
   /// \see AppendFragment - append a string.
   ///
   void AppendFragmentFromChars(const char* buff)
   {
      AppendFragmentFromBuffer(buff, static_cast<unsigned>(strlen(buff)));
   }

   /// Remove all children of the node.
   ///
   /// \see HasChildren - test whether the node has children.
   ///
   void RemoveAllChildren();

   /// Remove a child node, do nothing if there is no such node.
   ///
   /// The memory is managed by XmlDocument, do not attempt to delete the node.
   ///
   /// \param nameOrNodeObject
   ///    This is either a name of the object, or it is the object itself.
   /// \return bool, true if the child was present.
   ///
   /// \see RemoveExistingChild - throws an exception if there is no such child.
   ///
   bool RemoveChild(const MVariant& nameOrNodeObject);

   /// Remove a child node by name, do nothing if there is no such node.
   ///
   /// The memory is managed by XmlDocument, do not attempt to delete the node.
   ///
   /// \param name
   ///    Name of the child.
   /// \return bool, true if the child was present.
   ///
   /// \see RemoveChild
   /// \see RemoveExistingChildByName - throws an exception if there is no such child.
   ///
   bool RemoveChildByName(const MStdString& name);

   /// Remove a child object, do nothing if there is no such node.
   ///
   /// The memory is managed by XmlDocument, do not attempt to delete the node.
   ///
   /// \param node
   ///    Object that needs to be removed.
   /// \return bool, true if the child was present.
   ///
   /// \see RemoveChild
   /// \see RemoveExistingChildByObject - throws an exception if there is no such child.
   ///
   bool RemoveChildByObject(MXmlNode* node);

   /// Remove an existing child node.
   ///
   /// Throw an exception if there is no such child.
   /// The memory is managed by XmlDocument, do not attempt to delete the node.
   ///
   /// \param nameOrNodeObject
   ///    This is either a name of the object, or it is the object itself.
   ///
   /// \see RemoveChild - return False if such child does not exist.
   ///
   void RemoveExistingChild(const MVariant& nameOrNodeObject);

   /// Remove an existing child node by name.
   ///
   /// Throw an exception if there is no such child.
   /// The memory is managed by XmlDocument, do not attempt to delete the node.
   ///
   /// \param name
   ///    Name of the child.
   ///
   /// \see RemoveChild - return False if such child does not exist.
   /// \see RemoveExistingChild
   ///
   void RemoveExistingChildByName(const MStdString& name);

   /// Remove an existing child object.
   ///
   /// Throw an exception if there is no such child.
   /// The memory is managed by XmlDocument, do not attempt to delete the node.
   ///
   /// \param node
   ///    Object that needs to be removed.
   ///
   /// \see RemoveChild - return False if such child does not exist.
   /// \see RemoveExistingChild
   ///
   void RemoveExistingChildByObject(MXmlNode* node);

   /// Return the node path from the root of XML document.
   ///
   /// The string returned comprises the full element path delimited by
   /// \refprop{MXmlDocument::GetPathDelimiter, MXmlDocument::PathDelimiter}.
   /// Here is what a typical path looks like, delimited by forward slashes, the default:
   ///
   /// \code
   ///    /document-element/record/subnod
   /// \endcode
   ///
   MStdString GetPath() const;

   /// Convenience function that returns elements by path.
   ///
   /// The path given should use the value of property
   /// \refprop{MXmlDocument::GetPathDelimiter, MXmlDocument::PathDelimiter}
   /// for delimiting its components.
   /// If such node does not exist, an error is returned.
   ///
   /// \param path
   ///    Path of the element of interest.
   /// \return MXmlNode the object.
   ///
   MXmlNode* GetFirstElementByPath(const MStdString& path) const;

   ///@{
   /// Access root document object from any child.
   ///
   /// The root document is always present.
   ///
   virtual MXmlDocument* GetRoot();
   const MXmlDocument* GetRootConst() const
   {
      return const_cast<MXmlNode*>(this)->GetRoot();
   }
   ///@}

   /// Access the main element of the document from any child.
   ///
   /// For a typical HTML document this would be an element with name "html".
   /// Document element is one of the children of the document root, the only child of type element.
   ///
   /// If the document is empty, null is returned as there is no document element present.
   ///
   MXmlNode* GetDocumentElement() const;

protected:
/// \cond SHOW_INTERNAL

   // There is no way to initialize node outside document
   //
   MXmlNode()
   {
   }

   // There is no way to copy node outside document
   //
   MXmlNode(const MXmlNode&)
   {
   }

   // There is no way to destroy node outside document
   //
   virtual ~MXmlNode()
   {
   }

   MXmlNode* DoAccessXmlNode(pugi::xml_node node) const;

   MXmlNode* DoAccessXmlNodeAfterAdd(pugi::xml_node node) const;

   void DoCheckAttributeAdded(pugi::xml_attribute attr);

   virtual pugi::xml_node DoAccessPugiNode() const;

/// \endcond SHOW_INTERNAL
private:

   M_DECLARE_CLASS(XmlNode)
};

/// DOM representation of XML document.
///
/// XML document object manages memory for all its children all by itself,
/// no attempts should be made to delete any child nodes.
///
/// The implementation of DOM interface is based on one of the fastest libraries, pugixml, which is freeware.
/// There is very little overhead above pugixml on memory, and no overhead on speed.
///
class M_CLASS MXmlDocument : public MXmlNode
{
   friend class M_CLASS MXmlNode;

public: // Types:

   /// Document parsing mode.
   ///
   enum ParseEnum
   {
      /// If processing instructions are added to the DOM tree when parsed.
      ///
      /// This flag is off by default.
      ///
      ParsePi = 0x0001,

      /// If comments are added to the DOM tree.
      ///
      /// This flag is off by default.
      ///
      ParseComments = 0x0002,

      /// If CDATA sections are added to the DOM tree.
      ///
      /// This flag is on by default.
      ///
      ParseCdata = 0x0004,

      /// If plain character data that consist only of whitespace are added to the DOM tree.
      ///
      /// This flag is off by default and turning it on usually results in slower parsing and more memory consumption.
      ///
      ParseWsPcdata = 0x0008,

      /// If character and entity references are expanded during parsing.
      ///
      /// This flag is on by default.
      ///
      ParseEscapes = 0x0010,

      /// If EOL (End Of Line) characters are normalized, converted to a single '\n' during parsing.
      ///
      /// This flag is on by default.
      ///
      ParseEol = 0x0020,

      /// If attribute values are normalized using CDATA normalization rules during parsing.
      ///
      /// This flag is on by default.
      ///
      ParseWconvAttribute = 0x0040,

      /// If attribute values are normalized using NMTOKENS normalization rules during parsing.
      ///
      /// This flag is off by default.
      ///
      ParseWnormAttribute = 0x0080,

      /// If document declaration is added to the DOM tree.
      ///
      /// This flag is off by default.
      ///
      ParseDeclaration = 0x0100,

      /// If document type declaration is added to the DOM tree.
      ///
      /// This flag is off by default.
      ///
      ParseDoctype = 0x0200,

      /// If character data that is the only child of the parent node and that only has blanks is added to the DOM tree.
      ///
      /// This flag is off by default, turning it on may result in slower parsing and more memory consumption.
      ///
      ParseWsPcdataSingle = 0x0400,

      /// If leading and trailing whitespace is to be removed from plain character data.
      ///
      /// This flag is off by default.
      ///
      ParseTrimPcdata = 0x0800,

      /// If plain character data that does not have a parent node is added to the DOM tree, and if an empty document
      /// is a valid document.
      ///
      /// This flag is off by default.
      ///
      ParseFragment = 0x1000,

   // masks that consist of many values:

      /// Minimal parsing mode (equivalent to turning all other flags off).
      ///
      /// Only elements and PCDATA sections are added to the DOM tree, no text conversions are performed.
      ///
      ParseMaskMinimal = 0x0000,

      /// Default parsing mode, active when parsing mode is not set explicitly.
      ///
      /// Elements, PCDATA and CDATA sections are added to the DOM tree, character/reference entities are expanded,
      /// End of Line characters are normalized, attribute values are normalized using CDATA normalization rules.
      ///
      ParseMaskDefault = ParseCdata | ParseEscapes | ParseWconvAttribute | ParseEol,

      /// Full parsing mode, all information is gathered from XML.
      ///
      /// Nodes of all types are added to the DOM tree, character/reference entities are expanded,
      /// End of Line characters are normalized, attribute values are normalized using CDATA normalization rules.
      ///
      ParseMaskFull = ParseMaskDefault | ParsePi | ParseComments | ParseDeclaration | ParseDoctype
   };

   /// Formatting flags applied when the XML is written.
   ///
   enum
   {
      /// Indent the nodes that are written to output stream with as many indentation strings as deep the node is in DOM tree.
      ///
      /// This flag is on by default.
      ///
      FormatIndent = 0x0001,

      /// Write encoding-specific BOM (Byte Order Mark) to the output stream.
      ///
      /// This flag is off by default.
      ///
      FormatWriteBom = 0x0002,

      /// Use raw output mode, no indentation and no line breaks.
      ///
      /// When this flag is set, FormatIndent is ignored. This flag is off by default.
      ///
      FormatRaw = 0x0004,

      /// Omit default XML declaration even if there is no declaration in the document.
      ///
      /// This flag is off by default.
      ///
      FormatNoDeclaration = 0x0008,

      /// Do not escape attribute values and PCDATA contents.
      ///
      /// This flag is off by default.
      ///
      FormatNoEscapes = 0x0010,

      /// Open file using text mode.
      ///
      /// This enables special character (i.e. new-line) conversions on some systems.
      /// This flag is off by default.
      ///
      FormatSaveFileText = 0x20,

      /// Write every attribute on a new line with appropriate indentation.
      ///
      /// This flag is off by default.
      ///
      FormatIndentAttributes = 0x40,

   // masks that consist of many values:

      /// The default format mask.
      ///
      /// Nodes are indented depending on their depth in the DOM tree.
      ///
      FormatMaskDefault = FormatIndent
   };

public: // Types:

   /// Create an empty XML document, ready to be read or populated manually.
   ///
   /// No child nodes are present.
   ///
   MXmlDocument();

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
   /// \param parseMask
   ///      Parse mask to use. If not given, use ParseMaskDefault. When the first parameter is an XML Document, this is ignored.
   ///
   explicit MXmlDocument(const MVariant& streamFilenameOrString, unsigned parseMask = ParseMaskDefault);

   /// Create an XML document from a buffer that contains XML text.
   ///
   /// An error can result from stream I/O, or if the document is malformed.
   ///
   /// \param buffer
   ///     Pointer to a buffer to parse.
   /// \param size
   ///     Size of the buffer.
   /// \param parseMask
   ///      Parse mask to use. If not given, use ParseMaskDefault.
   ///
   MXmlDocument(const void* buffer, unsigned size, unsigned parseMask = ParseMaskDefault); // SWIG_HIDE

   /// Destructor, claims all memory allocated for children.
   ///
   virtual ~MXmlDocument();

   ///@{
   /// File name of the document, if the document was loaded from file.
   ///
   /// The file name is updated by Read services,
   /// or it can be set explicitly by the user of the class.
   /// Write services do not update this property.
   ///
   const MStdString& GetFileName() const
   {
      return m_fileName;
   }
   void SetFileName(const MStdString& name)
   {
      m_fileName = name;
   }
   ///@}

   ///@{
   /// Parse mask used during reading of XML.
   ///
   /// This is defined as a set of Parse constants. Default value is ParseMaskDefault.
   ///
   unsigned GetParseMask() const
   {
      return m_parseMask;
   }
   void SetParseMask(unsigned mask)
   {
      m_parseMask = mask;
   }
   ///@}

   ///@{
   /// Format mask used during writing of XML.
   ///
   /// This is defined as a set of Format constants. Default value is FormatMaskDefault.
   ///
   unsigned GetFormatMask() const
   {
      return m_formatMask;
   }
   void SetFormatMask(unsigned mask)
   {
      m_formatMask = mask;
   }
   ///@}

   ///@{
   /// Access indentation sequence, whatever is used to indent elements.
   ///
   /// By default, indentation sequence is three blanks.
   ///
   const MStdString& GetIndentationSequence() const
   {
      return m_indentationSequence;
   }
   void SetIndentationSequence(const MStdString& sequence)
   {
      m_indentationSequence = sequence;
   }
   ///@}

   ///@{
   /// Delimited character used for path construction.
   ///
   /// Typical values are slash, back slash, dot, or colon.
   /// Default value is '/'.
   ///
   char GetPathDelimiter() const
   {
      return m_pathDelimiter;
   }
   void SetPathDelimiter(char delimiter)
   {
      m_pathDelimiter = delimiter;
   }
   ///@}

   /// Read an XML document using a generic parameter.
   ///
   /// The previous contents of XML Document will be lost, pointers to nodes invalidated.
   /// An error can result from stream I/O, or in case the document is malformed.
   /// The property \refprop{GetParseMask, ParseMask} is used during parsing in order to determine
   /// which parts of the document are significant and should stay in memory.
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
   void Read(const MVariant& streamFilenameOrString);

   /// Read an XML document from a given string.
   ///
   /// The previous contents of XML Document will be lost, pointers to nodes invalidated.
   /// An error can result from bad contents of the string parameter.
   /// The property \refprop{GetParseMask, ParseMask} is used during parsing in order to determine
   /// which parts of the document are significant and should stay in memory.
   ///
   /// \param xmlString
   ///       An in-place XML document in a possibly long string.
   ///
   void ReadFromString(const MStdString& xmlString);

   /// Read an XML document from an opened stream.
   ///
   /// The previous contents of XML Document will be lost, pointers to nodes invalidated.
   /// An error can result from stream I/O, or if the document is malformed.
   /// The property \refprop{GetParseMask, ParseMask} is used during parsing in order to determine
   /// which parts of the document are significant and should stay in memory.
   ///
   /// \param stream
   ///     Stream object that is opened and ready to be read.
   ///     The whole stream will be read, but there will be no attempt to close the stream.
   ///
   void ReadFromStream(MStream* stream);

   /// Read an XML document from a file given by its name.
   ///
   /// The previous contents of XML Document will be lost, pointers to nodes invalidated.
   /// An error can result from stream I/O, or if the document is malformed.
   /// The property \refprop{GetParseMask, ParseMask} is used during parsing in order to determine
   /// which parts of the document are significant and should stay in memory.
   ///
   /// \param fileName
   ///     A file name from which to read the document.
   ///
   void ReadFromFile(const MStdString& fileName);

   /// Read an XML document from a given string.
   ///
   /// \param buffer
   ///     Buffer from which an XML document should be read.
   /// \param size
   ///     Size of the buffer.
   ///
   /// \see ReadFromString
   ///
   void ReadFromBuffer(const void* buffer, unsigned size);

   /// Read an XML document from a given string.
   ///
   /// \param buff
   ///     Zero terminated string from which an XML document should be read.
   ///
   /// \see ReadFromString
   ///
   void ReadFromChars(const char* buff)
   {
      ReadFromBuffer(buff, static_cast<unsigned>(strlen(buff)));
   }

   /// Clear all contents of this document.
   ///
   /// Delete all children, reclaim memory. All node pointers of this document will become invalid.
   ///
   void Clear();

   /// Write the contents of the document using a generic parameter.
   ///
   /// The properties \refprop{GetFormatMask, FormatMask} and \refprop{GetIndentationSequence, IndentationSequence}
   /// are used during the write.
   ///
   /// \param streamOrFilename
   ///     This can be:
   ///       - A stream object into which the XML document should be written.
   ///         The stream should be open outside, and there is no attempt made to close it inside the call.
   ///       - File name into which to write this document.
   ///
   /// \see AsString - an easy way of creating an XML context as a string.
   ///
   void Write(const MVariant& streamOrFilename);

   /// Write the contents of the document into a stream.
   ///
   /// The stream should be open outside, and there is no attempt made to close it inside the call.
   /// The properties \refprop{GetFormatMask, FormatMask} and \refprop{GetIndentationSequence, IndentationSequence}
   /// are used during the write.
   ///
   /// \param stream
   ///     Stream object into which the XML document should be written.
   ///
   /// \see AsString - an easy way of creating an XML context as a string.
   ///
   void WriteToStream(MStream* stream);

   /// Write the contents of the document into a file with a given name.
   ///
   /// The file will be created if it does not exist, or it will be truncated if there is already.
   /// The properties \refprop{GetFormatMask, FormatMask} and \refprop{GetIndentationSequence, IndentationSequence}
   /// are used during the write.
   ///
   /// \param fileName
   ///     File name into which to write this document.
   ///
   /// \see AsString - an easy way of creating an XML context as a string.
   ///
   void WriteToFile(const MStdString& fileName);

   ///@{
   /// Access root document object.
   ///
   /// In case of this call, document object, self is returned.
   ///
   virtual MXmlDocument* GetRoot();
   const MXmlDocument* GetRootConst() const
   {
      return const_cast<MXmlDocument*>(this)->GetRoot();
   }
   ///@}

   ///@{
   /// Copy the given document into self.
   ///
   /// The result document has a copy of all nodes of the given document.
   ///
   /// \param other
   ///     The document from which the value should be copied.
   ///
   void Assign(const MXmlDocument& other);
   MXmlDocument& operator=(const MXmlDocument& other)
   {
      Assign(other);
      return *this;
   }
   ///@}

protected:
/// \cond SHOW_INTERNAL

   virtual pugi::xml_node DoAccessPugiNode() const;

   void DoHandleParseResult(pugi::xml_parse_result& result, const char* text);

   // Copy constructor is MXmlDocument(const MVariant& other)
   MXmlDocument(const MXmlDocument& other);

/// \endcond SHOW_INTERNAL
private: // Data:

   // Pugi node handle. It has to be the first member!
   //
   pugi::xml_document m_document;

   // File name of the stream, if present
   //
   MStdString m_fileName;

   // Parse mask used during reading of the document,
   // corresponds to pugi parse mask.
   //
   unsigned m_parseMask;

   // Write format mask as used during writing of XML file, Format masks.
   // Corresponds to pugi format mask.
   //
   unsigned m_formatMask;

   // Indentation sequence, default is three blanks
   //
   MStdString m_indentationSequence;

   // Path delimiter
   //
   char m_pathDelimiter;

   M_DECLARE_CLASS(XmlDocument)
};

#endif // !M_NO_XML

///@}

/*
 * PARTS OF THE ABOVE TEXT, SUCH AS COMMENTS,
 * ARE COPIED FROM AND COPYRIGHTED BY Arseny Kapoulkine.
 * USED UNDER PERMISSIVE LICENSE.
 *
 * Copyright (c) 2006-2015 Arseny Kapoulkine
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#endif
