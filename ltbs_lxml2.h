

#ifndef LTBS_LXML2_H
#define LTBS_LXML2_H

#include <libxml/tree.h>
#include <libxml/HTMLparser.h>
#include <libxml/xpath.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct ltbs_xml_vt ltbs_xml_vt;
typedef void (*node_each_fn)(xmlNodePtr node, void *data);
typedef xmlNodePtr (*node_map_fn)(xmlNodePtr node, void *data);

struct ltbs_xml_vt
{
    htmlDocPtr (*from_file)(char *filepath);
    htmlDocPtr (*cstring_to_xml)(char *cstring);
    xmlXPathObjectPtr (*xpath)(htmlDocPtr doc, char *query);
    void (*node_each)(xmlXPathObjectPtr nodes, node_each_fn callback, void *data);
    xmlXPathObjectPtr (*node_map)(xmlXPathObjectPtr nodes, node_map_fn callback, void *data);
    char *(*node_get_text)(xmlNodePtr node);
    char *(*node_get_prop)(xmlNodePtr node, char *propname);
};

extern struct ltbs_xml_vt Libxml2_vt;

#endif // LTBS_LXML2_H

#define LTBS_LIBXML2_IMPLEMENTATION
#ifdef LTBS_LIBXML2_IMPLEMENTATION

htmlDocPtr from_file(char *filepath);
htmlDocPtr cstring_to_xml(char *cstring);
xmlXPathObjectPtr xpath_query(htmlDocPtr doc, char *query);
void node_each(xmlXPathObjectPtr nodes, node_each_fn callback, void *data);
xmlXPathObjectPtr node_map(xmlXPathObjectPtr nodes, node_map_fn callback, void *data);
char *node_get_text(xmlNodePtr node);
char *node_get_prop(xmlNodePtr node, char *propname);

ltbs_xml_vt Libxml2_vt = (ltbs_xml_vt)
{
    .from_file = from_file,
    .cstring_to_xml = cstring_to_xml,
    .xpath = xpath_query,
    .node_each = node_each,
    .node_map = node_map,
    .node_get_text = node_get_text,
    .node_get_prop = node_get_prop
};

htmlDocPtr from_file(char *filepath)
{
    int options =
	HTML_PARSE_RECOVER   |
	HTML_PARSE_NOIMPLIED |
	HTML_PARSE_NOERROR   |
	HTML_PARSE_NONET;

    htmlParserCtxtPtr html_parser = htmlNewParserCtxt();
    
    htmlDocPtr result = htmlCtxtReadFile(html_parser, filepath, "UTF-8", options);

    if ( result == NULL )
    {
	htmlFreeParserCtxt(html_parser);
	fprintf(stderr, "ERROR: Unable to load file %s\n", filepath);
	return NULL;
    }
    
    htmlFreeParserCtxt(html_parser);

    return result;
}

htmlDocPtr cstring_to_xml(char *cstring)
{
    htmlDocPtr result;
    htmlParserCtxtPtr html_parser = htmlNewParserCtxt();

    int options =
	HTML_PARSE_RECOVER   |
	HTML_PARSE_NOIMPLIED |
	HTML_PARSE_NOERROR   |
	HTML_PARSE_NONET;

    int64_t length = 0;

    for ( int64_t index = 0; cstring[index]; index++ ) { length++; }

    result = htmlCtxtReadMemory(html_parser, cstring, length, NULL, "UTF-8", options);
    htmlFreeParserCtxt(html_parser);

    return result;
}

xmlXPathObjectPtr xpath_query(htmlDocPtr doc, char *query)
{
    xmlXPathObjectPtr result;
    xmlXPathContextPtr context = xmlXPathNewContext(doc);

    if ( context == NULL )
    {
	fprintf(stderr, "ERROR: Failed to create xpath context.\n");
	return NULL;
    }

    result = xmlXPathEvalExpression((xmlChar *)query, context);
    xmlXPathFreeContext(context);

    if ( result == NULL )
    {
	fprintf(stderr, "Error: Failed to evaluate xpath expression.\nExpression: %s", query);
	return NULL;
    }

    if ( xmlXPathNodeSetIsEmpty(result->nodesetval) )
    {
	xmlXPathFreeObject(result);
	return NULL;
    }

    return result;
}

void node_each(xmlXPathObjectPtr nodes, node_each_fn callback, void *data)
{
    xmlNodeSetPtr nodeset = nodes->nodesetval;

    if ( nodeset->nodeNr == 0 ) return;

    for ( int index = 0; index < nodeset->nodeNr; index++ )
    {
	callback(nodeset->nodeTab[index], data);
    };
}

xmlXPathObjectPtr node_map(xmlXPathObjectPtr nodes, node_map_fn callback, void *data)
{
    xmlXPathObjectPtr result;
    xmlNodeSetPtr nodeset;
    
    if ( nodes == NULL ) return NULL;
    if ( nodes->nodesetval->nodeNr == 0 ) return NULL;

    result = xmlXPathObjectCopy(nodes);
    nodeset = result->nodesetval;

    for ( int index = 0; index < nodeset->nodeNr; index++ )
    {
	nodeset->nodeTab[index] = callback(nodeset->nodeTab[index], data);
    }

    return result;
}

char *node_get_text(xmlNodePtr node) { return (char *) xmlNodeGetContent(node); }

char *node_get_prop(xmlNodePtr node, char *propname) { return (char *) xmlGetProp(node, propname); }

#endif // LTBS_LIBXML2_IMPLEMENTATION
