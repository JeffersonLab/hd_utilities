/*
 *  hddm-py :    tool that reads in a HDDM document (Hall D Data Model)
 *        and writes a c++ class library that expresses the model as a
 *        python extension module. It does this by wrapping the classes
 *        of the c++ API as python classes, adding convenience methods
 *        to provide natural pythonic semantics for handling hddm files
 *        and objects.
 *
 *  author: richard.t.jones at uconn.edu
 *  version: june 24, 2016 - original release.
 *
 */

#include "XString.hpp"
#include "XParsers.hpp"
#include <xercesc/util/XMLUri.hpp>

#include <particleType.h>
#include <errno.h>

#include <string>
#include <vector>
#include <map>
#include <fstream>

#define X(str) XString(str).unicode_str()
#define S(str) str.c_str()

using namespace xercesc;

XString classPrefix;

void usage()
{
   std::cerr
        << "\nUsage:\n"
        << "    hddm-py [-v | -o <filename>] {HDDM file}\n\n"
        << "Options:\n"
        <<  "    -v            validate only\n"
        <<  "    -o <filename>    write to <filename>.cpy"
        << std::endl;
}

std::string guessType(const std::string &literal);
Particle_t lookupParticle(const std::string &name);

class XtString : public XString
{
/* XString class with a few extra methods for creating type
 * strings that are useful in creating class names
 */
 public:
   XtString() {};
   XtString(const char* s): XString(s) {};
   XtString(const XMLCh* p): XString(p) {};
   XtString(const std::string& s): XString(s) {};
   XtString(const XString& x): XString(x) {};
   XtString(const XtString& t): XString((XString&)t) {};
   ~XtString() {};

   XtString plural();
   XtString simpleType();
   XtString listType();
   XtString linkType();
};

class CodeBuilder
{
/* The methods in this class are used to write the c++ code
 * that implements the hddm python extension library.
 */
 public:
   std::ofstream pyFile;

   CodeBuilder() {};
   ~CodeBuilder() {};

   void checkConsistency(DOMElement* el, DOMElement* elref);
   void writeClassdef(DOMElement* el);
   void writeClassimp(DOMElement* el);
   void constructDocument(DOMElement* el);
   void constructGroup(DOMElement* el);
   void constructIOstreams(DOMElement* el);
   void constructMethods(DOMElement* el);
   void constructStreamers(DOMElement* el);
   void writeStreamers(DOMElement* el);

   typedef struct {
      std::string name;
      std::string args;
      std::string docs;
   } method_descr;

   std::map<XtString,XtString> typesList;

 private:
   std::vector<DOMElement*> tagList;
   typedef std::vector<DOMNode*> parentList_t;
   typedef std::map<const XtString,parentList_t> parentTable_t;
   parentList_t parentList;
   parentTable_t parents;
   parentTable_t children;
   int element_in_list(XtString &name, parentList_t list);
};


int main(int argC, char* argV[])
{
   try
   {
      XMLPlatformUtils::Initialize();
   }
   catch (const XMLException* toCatch)
   {
      XtString msg(toCatch->getMessage());
      std::cerr
           << "hddm-py: Error during initialization! :\n"
           << msg << std::endl;
      return 1;
   }

   if (argC < 2)
   {
      usage();
      return 1;
   }
   else if ((argC == 2) && (strcmp(argV[1], "-?") == 0))
   {
      usage();
      return 2;
   }

   XtString xmlFile;
   XtString pyFilename;
   bool verifyOnly = false;
   int argInd;
   for (argInd = 1; argInd < argC; argInd++)
   {
      if (argV[argInd][0] != '-')
      {
         break;
      }
      if (strcmp(argV[argInd],"-v") == 0)
      {
         verifyOnly = true;
      }
      else if (strcmp(argV[argInd],"-o") == 0)
      {
         pyFilename = XtString(argV[++argInd]);
      }
      else
      {
         std::cerr
              << "Unknown option \'" << argV[argInd]
              << "\', ignoring it\n" << std::endl;
      }
   }

   if (argInd != argC - 1)
   {
      usage();
      return 1;
   }
   xmlFile = XtString(argV[argInd]);

#if defined OLD_STYLE_XERCES_PARSER
   DOMDocument* document = parseInputDocument(xmlFile.c_str(),false);
#else
   DOMDocument* document = buildDOMDocument(xmlFile.c_str(),false);
#endif
   if (document == 0)
   {
      std::cerr
           << "hddm-py : Error parsing HDDM document, "
           << "cannot continue" << std::endl;
      return 1;
   }

   DOMElement* rootEl = document->getDocumentElement();
   XtString rootS(rootEl->getTagName());
   if (rootS != "HDDM")
   {
      std::cerr
           << "hddm-py error: root element of input document is "
           << "\"" << rootS << "\", expected \"HDDM\""
           << std::endl;
      return 1;
   }

   XtString classS(rootEl->getAttribute(X("class")));
   classPrefix = classS;

   XtString pyname;
   if (verifyOnly)
   {
      pyname = "/dev/null";
   }
   else if (pyFilename.size())
   {
      pyname = pyFilename + ".cpy";
   }
   else
   {
      pyname = "pyhddm_" + classPrefix + ".cpy";
   }

   CodeBuilder builder;
   builder.pyFile.open(pyname.c_str());
   if (! builder.pyFile.is_open())
   {
      std::cerr
           << "hddm-py error: unable to open output file "
           << pyname << std::endl;
      return 1;
   }

   builder.pyFile <<
   "/*\n"
   " * pyhddm_" << classPrefix << ".cpy - DO NOT EDIT THIS FILE\n"
   " *\n"
   " * This file was generated automatically by hddm-py from the file\n"
   << " * " << xmlFile << std::endl <<
   "\n"
   " * This source file contains the Python/C++ API wrappers that\n"
   " * provide a python interface to the hddm classes and methods\n"
   " * generated by hddm-cpp from " << xmlFile << ".\n"
   " *\n"
   " * The hddm data model tool set was written by\n"
   " * Richard Jones, University of Connecticut.\n"
   " *\n"
   " * For more information see the following web site\n"
   " *\n"
   " * http://zeus.phys.uconn.edu/halld/datamodel/doc\n"
   " *\n"
   " */\n"
   "\n"
   "#include <Python.h>\n"
   "#include <structmember.h>\n"
   "\n"
   "#include <hddm_" << classPrefix << ".hpp>\n"
   "#include <fstream>\n"
   "#include <iostream>\n"
   "#include <exception>\n"
   "#include <particleType.h>\n"
   "\n"
   "using namespace hddm_" << classPrefix << ";\n"
   "\n"
   "#if PY_MAJOR_VERSION >= 3\n"
   "   #define PyInt_FromLong PyLong_FromLong\n"
   "   #define PyInt_AsLong PyLong_AsLong\n"
   "#endif\n"
   "\n"
   "\n"
   "inline void LOG_NEW(PyTypeObject *t, PyTypeObject *subt=0, int own=0) {\n"
   "#if 0\n"
   "   if (subt == 0)\n"
   "      std::cout << \"creating a new element of \" << t->tp_name\n"
   "                << \" \" << ((own == 0)? \"(borrowed)\" : \"(owned)\")\n"
   "                << std::endl;\n"
   "   else\n"
   "      std::cout << \"creating a new list of \" << subt->tp_name\n"
   "                << \" \" << ((own == 0)? \"(borrowed)\" : \"(owned)\")\n"
   "                << std::endl;\n"
   "#endif\n"
   "}\n"
   "\n"
   "inline void LOG_DEALLOC(PyTypeObject *t, PyTypeObject *subt=0, int own=0) {\n"
   "#if 0\n"
   "   if (subt == 0)\n"
   "      std::cout << \"destroying an element of \" << t->tp_name\n"
   "                << \" \" << ((own == 0)? \"(borrowed)\" : \"(owned)\")\n"
   "                << std::endl;\n"
   "   else\n"
   "      std::cout << \"destroying a list of \" << subt->tp_name\n"
   "                << \" \" << ((own == 0)? \"(borrowed)\" : \"(owned)\")\n"
   "                << std::endl;\n"
   "#endif\n"
   "}\n"
   "\n"
   "inline void My_INCREF(PyObject *o) {\n"
   "   //std::cout << \"incrementing reference at \" << o << std::endl;\n"
   "   Py_INCREF(o);\n"
   "}\n"
   "\n"
   "inline void My_DECREF(PyObject *o) {\n"
   "   //std::cout << \"decrementing reference at \" << o << std::endl;\n"
   "   Py_DECREF(o);\n"
   "}\n"
   "\n"
   "// wrap base class hddm_" << classPrefix << "::HDDM_Element"
   " as hddm_" << classPrefix << ".HDDM_Element\n"
   "\n"
   "typedef struct {\n"
   "   PyObject_HEAD\n"
   "   HDDM_Element *elem;\n"
   "   PyObject *host;\n"
   "} _HDDM_Element;\n"
   "\n"
   "static void\n"
   "_HDDM_Element_dealloc(_HDDM_Element* self)\n"
   "{\n"
   "   if (self->elem != 0) {\n"
   "      LOG_DEALLOC(Py_TYPE(self), 0, self->host == (PyObject*)self);\n"
   "      if (self->host == (PyObject*)self)\n"
   "         delete self->elem;\n"
   "      else\n"
   "         My_DECREF(self->host);\n"
   "   }\n"
   "   Py_TYPE(self)->tp_free((PyObject*)self);\n"
   "}\n"
   "\n"
   "static PyObject*\n"
   "_HDDM_Element_new(PyTypeObject *type, PyObject *args, PyObject *kwds)\n"
   "{\n"
   "   _HDDM_Element *self;\n"
   "   self = (_HDDM_Element*)type->tp_alloc(type, 0);\n"
   "   if (self != NULL) {\n"
   "      self->elem = 0;\n"
   "      self->host = 0;\n"
   "   }\n"
   "   return (PyObject*)self;\n"
   "}\n"
   "\n"
   "static int\n"
   "_HDDM_Element_init(_HDDM_Element *self, PyObject *args, PyObject *kwds)\n"
   "{\n"
   "   PyErr_SetString(PyExc_RuntimeError, \"illegal constructor\");\n"
   "   return -1;\n"
   "}\n"
   "\n"
   "static PyObject*\n"
   "_HDDM_Element_getAttribute(PyObject *self, PyObject *args)\n"
   "{\n"
   "   char *attr;\n"
   "   if (! PyArg_ParseTuple(args, \"s\", &attr)) {\n"
   "      return NULL;\n"
   "   }\n"
   "   _HDDM_Element *me = (_HDDM_Element*)self;\n"
   "   if (me->elem == 0) {\n"
   "      PyErr_SetString(PyExc_RuntimeError, \"lookup attempted on invalid"
   " element\");\n"
   "      return NULL;\n"
   "   }\n"
   "   hddm_type atype;\n"
   "   void *val((int*)me->elem->getAttribute(std::string(attr),&atype));\n"
   "   if (val == 0) {\n"
   "      Py_INCREF(Py_None);\n"
   "      return Py_None;\n"
   "   }\n"
   "   else if (atype == k_hddm_int) {\n"
   "      return PyLong_FromLong(*(int*)val);\n"
   "   }\n"
   "   else if (atype == k_hddm_long) {\n"
   "      return PyLong_FromLong(*(long*)val);\n"
   "   }\n"
   "   else if (atype == k_hddm_float) {\n"
   "      return PyFloat_FromDouble(double(*(float*)val));\n"
   "   }\n"
   "   else if (atype == k_hddm_double) {\n"
   "      return PyFloat_FromDouble(*(double*)val);\n"
   "   }\n"
   "   else if (atype == k_hddm_boolean) {\n"
   "      if (*(bool*)val == 0) {\n"
   "         Py_INCREF(Py_False);\n"
   "         return Py_False;\n"
   "      }\n"
   "      else {\n"
   "         Py_INCREF(Py_True);\n"
   "         return Py_True;\n"
   "      }\n"
   "   }\n"
   "   else if (atype == k_hddm_string) {\n"
   "      return PyUnicode_FromString(((std::string*)val)->c_str());\n"
   "   }\n"
   "   else if (atype == k_hddm_anyURI) {\n"
   "      return PyUnicode_FromString(((std::string*)val)->c_str());\n"
   "   }\n"
   "   else if (atype == k_hddm_Particle_t) {\n"
   "      return PyUnicode_FromString(ParticleType(*(Particle_t*)val));\n"
   "   }\n"
   "   return PyUnicode_FromString(((std::string*)val)->c_str());\n"
   "}\n\n"
   "static PyMemberDef _HDDM_Element_members[] = {\n"
   "   {NULL}  /* Sentinel */\n"
   "};\n"
   "\n"
   "static PyMethodDef _HDDM_Element_methods[] = {\n"
   "   {\"getAttribute\", _HDDM_Element_getAttribute, METH_VARARGS,\n"
   "    \"look up named attribute in this element\"},\n"
   "   {NULL}  /* Sentinel */\n"
   "};\n"
   "\n"
   "static PyTypeObject _HDDM_Element_type = {\n"
   "    PyVarObject_HEAD_INIT(NULL,0)\n"
   "    \"hddm_" << classPrefix << ".HDDM_Element\",     /*tp_name*/\n"
   "    sizeof(_HDDM_Element),     /*tp_basicsize*/\n"
   "    0,                         /*tp_itemsize*/\n"
   "    (destructor)_HDDM_Element_dealloc, /*tp_dealloc*/\n"
   "    0,                         /*tp_print*/\n"
   "    0,                         /*tp_getattr*/\n"
   "    0,                         /*tp_setattr*/\n"
   "    0,                         /*tp_compare*/\n"
   "    0,                         /*tp_repr*/\n"
   "    0,                         /*tp_as_number*/\n"
   "    0,                         /*tp_as_sequence*/\n"
   "    0,                         /*tp_as_mapping*/\n"
   "    0,                         /*tp_hash */\n"
   "    0,                         /*tp_call*/\n"
   "    0,                         /*tp_str*/\n"
   "    0,                         /*tp_getattro*/\n"
   "    0,                         /*tp_setattro*/\n"
   "    0,                         /*tp_as_buffer*/\n"
   "    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/\n"
   "    \"hddm_" << classPrefix << " basic element\",    /* tp_doc */\n"
   "    0,                         /* tp_traverse */\n"
   "    0,                         /* tp_clear */\n"
   "    0,                         /* tp_richcompare */\n"
   "    0,                         /* tp_weaklistoffset */\n"
   "    0,                         /* tp_iter */\n"
   "    0,                         /* tp_iternext */\n"
   "    _HDDM_Element_methods,     /* tp_methods */\n"
   "    _HDDM_Element_members,     /* tp_members */\n"
   "    0,                         /* tp_getset */\n"
   "    0,                         /* tp_base */\n"
   "    0,                         /* tp_dict */\n"
   "    0,                         /* tp_descr_get */\n"
   "    0,                         /* tp_descr_set */\n"
   "    0,                         /* tp_dictoffset */\n"
   "    (initproc)_HDDM_Element_init, /* tp_init */\n"
   "    0,                         /* tp_alloc */\n"
   "    _HDDM_Element_new,         /* tp_new */\n"
   "};\n"
   "\n"
   "\n"
   "// wrap base class hddm_" << classPrefix << "::HDDM_ElementList"
   " as hddm_" << classPrefix << ".HDDM_ElementList\n"
   "\n"
   "typedef struct {\n"
   "   PyObject_HEAD\n"
   "   PyTypeObject *subtype; // type of wrapper derived from _HDDM_Element\n"
   "   HDDM_ElementList<HDDM_Element> *list;\n"
   "   PyObject *host;\n"
   "   int borrowed;\n"
   "} _HDDM_ElementList;\n"
   "\n"
   "static void\n"
   "_HDDM_ElementList_dealloc(_HDDM_ElementList* self)\n"
   "{\n"
   "   if (self->list != 0) {\n"
   "      LOG_DEALLOC(Py_TYPE(self), self->subtype, self->borrowed == 0);\n"
   "      if (self->borrowed == 0)\n"
   "         delete self->list;\n"
   "      My_DECREF(self->host);\n"
   "   }\n"
   "   Py_TYPE(self)->tp_free((PyObject*)self);\n"
   "}\n"
   "\n"
   "static PyObject*\n"
   "_HDDM_ElementList_new(PyTypeObject *type, PyObject *args, PyObject *kwds)\n"
   "{\n"
   "   _HDDM_ElementList *self;\n"
   "   self = (_HDDM_ElementList*)type->tp_alloc(type, 0);\n"
   "   if (self != NULL) {\n"
   "      self->subtype = 0;\n"
   "      self->borrowed = 0;\n"
   "      self->host = 0;\n"
   "   }\n"
   "   return (PyObject*)self;\n"
   "}\n"
   "\n"
   "static int\n"
   "_HDDM_ElementList_init(_HDDM_ElementList *self, PyObject *args, PyObject *kwds)\n"
   "{\n"
   "   PyErr_SetString(PyExc_RuntimeError, \"illegal constructor\");\n"
   "   return -1;\n"
   "}\n"
   "\n"
   "static Py_ssize_t\n"
   "_HDDM_ElementList_size(_HDDM_ElementList *self, void *closure)\n"
   "{\n"
   "   if (self->list == 0) {\n"
   "      PyErr_SetString(PyExc_RuntimeError, \"size attempted on invalid list\");\n"
   "      return -1;\n"
   "   }\n"
   "   return self->list->size();\n"
   "}\n"
   "\n"
   "static PyObject*\n"
   "_HDDM_ElementList_item(_HDDM_ElementList *self, Py_ssize_t i)\n"
   "{\n"
   "   if (self->list == 0)\n"
   "      return NULL;\n"
   "   int len = self->list->size();\n"
   "   if (i < 0 || i >= len) {\n"
   "      PyErr_Format(PyExc_IndexError, \"index \\%ld out of bounds.\", i);\n"
   "      return NULL;\n"
   "   }\n"
   "   PyObject *elem_obj = _HDDM_Element_new(self->subtype, 0, 0);\n"
   "   ((_HDDM_Element*)elem_obj)->elem = &(HDDM_Element&)(*self->list)(i);\n"
   "   ((_HDDM_Element*)elem_obj)->host = self->host;\n"
   "   My_INCREF(self->host);\n"
   "   LOG_NEW(self->subtype);\n"
   "   return elem_obj;\n"
   "}\n"
   "\n"
   "extern PyTypeObject _HDDM_ElementList_type;\n"
   "\n"
   "static PyObject *\n"
   "_HDDM_ElementList_add(PyObject *self, PyObject *args)\n"
   "{\n"
   "   int count=0;\n"
   "   int start=-1;\n"
   "   if (! PyArg_ParseTuple(args, \"i|i\", &count, &start)) {\n"
   "      return NULL;\n"
   "   }\n"
   "   _HDDM_ElementList *me = (_HDDM_ElementList*)self;\n"
   "   if (me->list == 0) {\n"
   "      PyErr_SetString(PyExc_RuntimeError, \"add attempted on invalid list\");\n"
   "      return NULL;\n"
   "   }\n"
   "   PyObject *list = _HDDM_ElementList_new(&_HDDM_ElementList_type, 0, 0);\n"
   "   ((_HDDM_ElementList*)list)->subtype = me->subtype;\n"
   "   ((_HDDM_ElementList*)list)->list = (HDDM_ElementList<HDDM_Element>*)\n"
   "    new HDDM_ElementList<HDDM_Element>(me->list->add(count, start));\n"
   "   ((_HDDM_ElementList*)list)->borrowed = 0;\n"
   "   ((_HDDM_ElementList*)list)->host = me->host;\n"
   "   My_INCREF(me->host);\n"
   "   LOG_NEW(Py_TYPE(self), me->subtype, 1);\n"
   "   return list;\n"
   "}\n"
   "\n"
   "static PyObject *\n"
   "_HDDM_ElementList_del(PyObject *self, PyObject *args)\n"
   "{\n"
   "   int start=0;\n"
   "   int count=-1;\n"
   "   if (! PyArg_ParseTuple(args, \"|ii\", &count, &start)) {\n"
   "      return NULL;\n"
   "   }\n"
   "   _HDDM_ElementList *list_obj;\n"
   "   list_obj = (_HDDM_ElementList*)self;\n"
   "   if (list_obj->list == 0) {\n"
   "      PyErr_SetString(PyExc_RuntimeError, \"del attempted on invalid list\");\n"
   "      return NULL;\n"
   "   }\n"
   "   list_obj->list->del(count, start);\n"
   "   Py_INCREF(self);\n"
   "   return self;\n"
   "}\n"
   "\n"
   "static PyObject *\n"
   "_HDDM_ElementList_clear(PyObject *self, PyObject *args)\n"
   "{\n"
   "   _HDDM_ElementList *list_obj;\n"
   "   list_obj = (_HDDM_ElementList*)self;\n"
   "   if (list_obj->list == 0) {\n"
   "      PyErr_SetString(PyExc_RuntimeError, \"clear attempted on invalid list\");\n"
   "      return NULL;\n"
   "   }\n"
   "   list_obj->list->clear();\n"
   "   Py_INCREF(self);\n"
   "   return self;\n"
   "}\n"
   "\n"
   "static PyMemberDef _HDDM_ElementList_members[] = {\n"
   "   {NULL}  /* Sentinel */\n"
   "};\n"
   "\n"
   "static PyMethodDef _HDDM_ElementList_methods[] = {\n"
   "   {\"add\",  _HDDM_ElementList_add, METH_VARARGS,\n"
   "    \"add (or insert) a new element to the list.\"},\n"
   "   {\"del\",  _HDDM_ElementList_del, METH_VARARGS,\n"
   "    \"delete an existing element from the list.\"},\n"
   "   {\"clear\",  _HDDM_ElementList_clear, METH_NOARGS,\n"
   "    \"reset the list to zero elements.\"},\n"
   "   {NULL}  /* Sentinel */\n"
   "};\n"
   "\n"
   "static PySequenceMethods _HDDM_ElementList_as_sequence = {\n"
   "    (lenfunc)_HDDM_ElementList_size,            /* sq_length */\n"
   "    0,                                          /* sq_concat */\n"
   "    0,                                          /* sq_repeat */\n"
   "    (ssizeargfunc)_HDDM_ElementList_item,       /* sq_item */\n"
   "    0,                                          /* sq_slice */\n"
   "    0,                                          /* sq_ass_item */\n"
   "    0,                                          /* sq_ass_slice */\n"
   "    0,                                          /* sq_contains */\n"
   "    0,                                          /* sq_inplace_concat */\n"
   "    0,                                          /* sq_inplace_repeat */\n"
   "};\n"
   "\n"
   "PyTypeObject _HDDM_ElementList_type = {\n"
   "    PyVarObject_HEAD_INIT(NULL,0)\n"
   "    \"hddm_" << classPrefix << ".HDDM_ElementList\", /*tp_name*/\n"
   "    sizeof(_HDDM_ElementList), /*tp_basicsize*/\n"
   "    0,                         /*tp_itemsize*/\n"
   "    (destructor)_HDDM_ElementList_dealloc, /*tp_dealloc*/\n"
   "    0,                         /*tp_print*/\n"
   "    0,                         /*tp_getattr*/\n"
   "    0,                         /*tp_setattr*/\n"
   "    0,                         /*tp_compare*/\n"
   "    0,                         /*tp_repr*/\n"
   "    0,                         /*tp_as_number*/\n"
   "    &_HDDM_ElementList_as_sequence, /*tp_as_sequence*/\n"
   "    0,                         /*tp_as_mapping*/\n"
   "    0,                         /*tp_hash */\n"
   "    0,                         /*tp_call*/\n"
   "    0,                         /*tp_str*/\n"
   "    0,                         /*tp_getattro*/\n"
   "    0,                         /*tp_setattro*/\n"
   "    0,                         /*tp_as_buffer*/\n"
   "    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/\n"
   "    \"hddm_" << classPrefix << " output stream\",    /* tp_doc */\n"
   "    0,                         /* tp_traverse */\n"
   "    0,                         /* tp_clear */\n"
   "    0,                         /* tp_richcompare */\n"
   "    0,                         /* tp_weaklistoffset */\n"
   "    0,                         /* tp_iter */\n"
   "    0,                         /* tp_iternext */\n"
   "    _HDDM_ElementList_methods, /* tp_methods */\n"
   "    _HDDM_ElementList_members, /* tp_members */\n"
   "    0,                         /* tp_getset */\n"
   "    0,                         /* tp_base */\n"
   "    0,                         /* tp_dict */\n"
   "    0,                         /* tp_descr_get */\n"
   "    0,                         /* tp_descr_set */\n"
   "    0,                         /* tp_dictoffset */\n"
   "    (initproc)_HDDM_ElementList_init,   /* tp_init */\n"
   "    0,                         /* tp_alloc */\n"
   "    _HDDM_ElementList_new,     /* tp_new */\n"
   "};\n"
   ;

   builder.constructGroup(rootEl);
   builder.constructIOstreams(rootEl);
   builder.constructMethods(rootEl);
   builder.constructStreamers(rootEl);

   builder.typesList["HDDM_Element"] = "_HDDM_Element_type";
   builder.typesList["HDDM_ElementList"] = "_HDDM_ElementList_type";
   builder.typesList["streamposition"] = "_streamposition_type";
   builder.typesList["ostream"] = "_ostream_type";
   builder.typesList["istream"] = "_istream_type";

   builder.pyFile <<
   "\n"
   "\n"
   "// wrap class hddm_" << classPrefix << "::streamposition"
   " as hddm_" << classPrefix << ".streamposition\n"
   "\n"
   "typedef struct {\n"
   "   PyObject_HEAD\n"
   "   streamposition *streampos;\n"
   "} _streamposition;\n"
   "\n"
   "static void\n"
   "_streamposition_dealloc(_streamposition* self)\n"
   "{\n"
   "   if (self->streampos != 0)\n"
   "      delete self->streampos;\n"
   "   Py_TYPE(self)->tp_free((PyObject*)self);\n"
   "}\n"
   "\n"
   "static PyObject*\n"
   "_streamposition_new(PyTypeObject *type, PyObject *args, PyObject *kwds)\n"
   "{\n"
   "   _streamposition *self;\n"
   "   self = (_streamposition*)type->tp_alloc(type, 0);\n"
   "   if (self != NULL)\n"
   "      self->streampos = 0;\n"
   "   return (PyObject*)self;\n"
   "}\n"
   "\n"
   "static int\n"
   "_streamposition_init(_streamposition *self, PyObject *args, PyObject *kwds)\n"
   "{\n"
   "   const char *kwlist[] = {\"start\", \"offset\", \"status\", NULL};\n"
   "   uint64_t start = 0;\n"
   "   uint32_t offset = 0;\n"
   "   uint32_t status = 0;\n"
   "   if (PyArg_ParseTuple(args, \"\") ||\n"
   "       PyArg_ParseTupleAndKeywords(args, kwds, \"kII\", (char**)kwlist, \n"
   "                                   &start, &offset, &status))\n"
   "   {\n"
   "      PyErr_Clear();\n"
   "      if (self->streampos != 0)\n"
   "         delete self->streampos;\n"
   "      self->streampos = new streamposition(start, offset, status);\n"
   "      return 0;\n"
   "   }\n"
   "   return -1; \n"
   "}\n"
   "\n"
   "static PyObject*\n"
   "_streamposition_richcompare(PyObject *a, PyObject *b, int op)\n"
   "{\n"
   "   int res = 0;\n"
   "   streamposition *apos = ((_streamposition*)a)->streampos;\n"
   "   streamposition *bpos = ((_streamposition*)b)->streampos;\n"
   "   if (op == Py_LT)\n"
   "      res = (*apos < *bpos);\n"
   "   else if (op == Py_LE)\n"
   "      res = (*apos <= *bpos);\n"
   "   else if (op == Py_EQ)\n"
   "      res = (*apos == *bpos);\n"
   "   else if (op == Py_NE)\n"
   "      res = (*apos != *bpos);\n"
   "   else if (op == Py_GT)\n"
   "      res = (*apos > *bpos);\n"
   "   else if (op == Py_GE)\n"
   "      res = (*apos >= *bpos);\n"
   "   if (res) {\n"
   "      Py_INCREF(Py_True);\n"
   "      return Py_True;\n"
   "   }\n"
   "   else {\n"
   "      Py_INCREF(Py_False);\n"
   "      return Py_False;\n"
   "   }\n"
   "}\n"
   "static PyObject*\n"
   "_streamposition_toString(PyObject *self, PyObject *args=0)\n"
   "{\n"
   "   std::stringstream ostr;\n"
   "   ostr << \"hddm_" << classPrefix << ".streamposition(\"\n"
   "        << ((_streamposition*)self)->streampos->block_start << \",\"\n"
   "        << ((_streamposition*)self)->streampos->block_offset << \",\"\n"
   "        << ((_streamposition*)self)->streampos->block_status\n"
   "        << \")\";\n"
   "   return PyUnicode_FromString(ostr.str().c_str());\n"
   "}\n"
   "\n"
   "static PyObject*\n"
   "_streamposition_toRepr(PyObject *self, PyObject *args=0)\n"
   "{\n"
   "   std::stringstream ostr;\n"
   "   ostr << \"\\\'\";\n"
   "   ostr << \"hddm_" << classPrefix << ".streamposition(\"\n"
   "        << ((_streamposition*)self)->streampos->block_start << \",\"\n"
   "        << ((_streamposition*)self)->streampos->block_offset << \",\"\n"
   "        << ((_streamposition*)self)->streampos->block_status\n"
   "        << \")\";\n"
   "   ostr << \"\\\'\";\n"
   "   return PyUnicode_FromString(ostr.str().c_str());\n"
   "}\n"
   "\n"
   "static PyObject*\n"
   "_streamposition_getstart(_streamposition *self, void *closure)\n"
   "{\n"
   "   return Py_BuildValue(\"k\", self->streampos->block_start);\n"
   "}\n"
   "\n"
   "static int\n"
   "_streamposition_setstart(_streamposition *self, PyObject *value, void *closure)\n"
   "{\n"
   "   if (value == NULL) {\n"
   "      PyErr_SetString(PyExc_TypeError, \"unexpected null argument\");\n"
   "      return -1;\n"
   "   }\n"
   "   long start = PyInt_AsLong(value);\n"
   "   if (start < 0 && PyErr_Occurred()) {\n"
   "      return -1;\n"
   "   }\n"
   "   self->streampos->block_start = start;\n"
   "   return 0;\n"
   "}\n"
   "\n"
   "static PyObject*\n"
   "_streamposition_getoffset(_streamposition *self, void *closure)\n"
   "{\n"
   "   return Py_BuildValue(\"I\", self->streampos->block_offset);\n"
   "}\n"
   "\n"
   "static int\n"
   "_streamposition_setoffset(_streamposition *self, PyObject *value, void *closure)\n"
   "{\n"
   "   if (value == NULL) {\n"
   "      PyErr_SetString(PyExc_TypeError, \"unexpected null argument\");\n"
   "      return -1;\n"
   "   }\n"
   "   long offset = PyInt_AsLong(value);\n"
   "   if (offset < 0 && PyErr_Occurred()) {\n"
   "      return -1;\n"
   "   }\n"
   "   self->streampos->block_offset = offset;\n"
   "   return 0;\n"
   "}\n"
   "\n"
   "static PyObject*\n"
   "_streamposition_getstatus(_streamposition *self, void *closure)\n"
   "{\n"
   "   return Py_BuildValue(\"I\", self->streampos->block_status);\n"
   "}\n"
   "\n"
   "static int\n"
   "_streamposition_setstatus(_streamposition *self, PyObject *value, void *closure)\n"
   "{\n"
   "   if (value == NULL) {\n"
   "      PyErr_SetString(PyExc_TypeError, \"unexpected null argument\");\n"
   "      return -1;\n"
   "   }\n"
   "   long status = PyInt_AsLong(value);\n"
   "   if (status == -1 && PyErr_Occurred()) {\n"
   "      return -1;\n"
   "   }\n"
   "   self->streampos->block_status = status;\n"
   "   return 0;\n"
   "}\n"
   "\n"
   "static PyGetSetDef _streamposition_getsetters[] = {\n"
   "   {(char*)\"start\", \n"
   "    (getter)_streamposition_getstart, (setter)_streamposition_setstart,\n"
   "    (char*)\"block start position\",\n"
   "    NULL},\n"
   "   {(char*)\"offset\", \n"
   "    (getter)_streamposition_getoffset, (setter)_streamposition_setoffset,\n"
   "    (char*)\"block offset position\",\n"
   "    NULL},\n"
   "   {(char*)\"status\", \n"
   "    (getter)_streamposition_getstatus, (setter)_streamposition_setstatus,\n"
   "    (char*)\"block status flags\",\n"
   "    NULL},\n"
   "   {NULL}  /* Sentinel */\n"
   "};\n"
   "\n"
   "static PyMemberDef _streamposition_members[] = {\n"
   "   {NULL}  /* Sentinel */\n"
   "};\n"
   "\n"
   "static PyMethodDef _streamposition_methods[] = {\n"
   "   {NULL}  /* Sentinel */\n"
   "};\n"
   "\n"
   "static PyTypeObject _streamposition_type = {\n"
   "   PyVarObject_HEAD_INIT(NULL,0)\n"
   "   \"hddm_" << classPrefix << ".streamposition\",   /*tp_name*/\n"
   "   sizeof(_streamposition),   /*tp_basicsize*/\n"
   "   0,                         /*tp_itemsize*/\n"
   "   (destructor)_streamposition_dealloc, /*tp_dealloc*/\n"
   "   0,                         /*tp_print*/\n"
   "   0,                         /*tp_getattr*/\n"
   "   0,                         /*tp_setattr*/\n"
   "   0,                         /*tp_compare*/\n"
   "   (reprfunc)_streamposition_toRepr, /*tp_repr*/\n"
   "   0,                         /*tp_as_number*/\n"
   "   0,                         /*tp_as_sequence*/\n"
   "   0,                         /*tp_as_mapping*/\n"
   "   0,                         /*tp_hash */\n"
   "   0,                         /*tp_call*/\n"
   "   (reprfunc)_streamposition_toString, /*tp_str*/\n"
   "   0,                         /*tp_getattro*/\n"
   "   0,                         /*tp_setattro*/\n"
   "   0,                         /*tp_as_buffer*/\n"
   "   Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/\n"
   "   \"hddm_" << classPrefix << " streamposition objects\", /* tp_doc */\n"
   "   0,                         /* tp_traverse */\n"
   "   0,                         /* tp_clear */\n"
   "   _streamposition_richcompare, /* tp_richcompare */\n"
   "   0,                         /* tp_weaklistoffset */\n"
   "   0,                         /* tp_iter */\n"
   "   0,                         /* tp_iternext */\n"
   "   _streamposition_methods,   /* tp_methods */\n"
   "   _streamposition_members,   /* tp_members */\n"
   "   _streamposition_getsetters, /* tp_getset */\n"
   "   0,                         /* tp_base */\n"
   "   0,                         /* tp_dict */\n"
   "   0,                         /* tp_descr_get */\n"
   "   0,                         /* tp_descr_set */\n"
   "   0,                         /* tp_dictoffset */\n"
   "   (initproc)_streamposition_init, /* tp_init */\n"
   "   0,                         /* tp_alloc */\n"
   "   _streamposition_new,       /* tp_new */\n"
   "};\n"
   "\n"
   "\n"
   "// wrap class hddm_" << classPrefix << "::ostream"
   " as hddm_" << classPrefix << ".ostream\n"
   "\n"
   "typedef struct {\n"
   "   PyObject_HEAD\n"
   "   std::string *fname;\n"
   "   std::ofstream *fstr;\n"
   "   ostream *ostr;\n"
   "} _ostream;\n"
   "\n"
   "static void\n"
   "_ostream_dealloc(_ostream* self)\n"
   "{\n"
   "   if (self->fname != 0)\n"
   "      delete self->fname;\n"
   "   if (self->ostr != 0)\n"
   "      delete self->ostr;\n"
   "   if (self->fstr != 0)\n"
   "      delete self->fstr;\n"
   "   Py_TYPE(self)->tp_free((PyObject*)self);\n"
   "}\n"
   "\n"
   "static PyObject*\n"
   "_ostream_new(PyTypeObject *type, PyObject *args, PyObject *kwds)\n"
   "{\n"
   "   _ostream *self;\n"
   "   self = (_ostream*)type->tp_alloc(type, 0);\n"
   "   if (self != NULL) {\n"
   "      self->fname = 0;\n"
   "      self->fstr = 0;\n"
   "      self->ostr = 0;\n"
   "   }\n"
   "   return (PyObject*)self;\n"
   "}\n"
   "\n"
   "static int\n"
   "_ostream_init(_ostream *self, PyObject *args, PyObject *kwds)\n"
   "{\n"
   "   const char *kwlist[] = {\"file\", NULL};\n"
   "   char *filename;\n"
   "   if (PyArg_ParseTupleAndKeywords(args, kwds, \"s\", (char**)kwlist, &filename))\n"
   "   {\n"
   "      if (self->fname != 0)\n"
   "         delete self->fname;\n"
   "      if (self->ostr != 0)\n"
   "         delete self->ostr;\n"
   "      if (self->fstr != 0)\n"
   "         delete self->fstr;\n"
   "      self->fname = new std::string(filename);\n"
   "      self->fstr = new std::ofstream(filename);\n"
   "      if (! self->fstr->good()) {\n"
   "         PyErr_Format(PyExc_IOError, \"Cannot open output file \\%s\", filename);\n"
   "         return -1;\n"
   "      }\n"
   "      try {\n"
   "         self->ostr = new ostream(*self->fstr);\n"
   "      }\n"
   "      catch (std::exception& e) {\n"
   "         PyErr_SetString(PyExc_RuntimeError, e.what());\n"
   "         return -1;\n"
   "      }\n"
   "      return 0;\n"
   "   }\n"
   "   return -1; \n"
   "}\n"
   "\n"
   "static PyObject*\n"
   "_ostream_getCompression(_ostream *self, void *closure)\n"
   "{\n"
   "   return Py_BuildValue(\"i\", self->ostr->getCompression());\n"
   "}\n"
   "\n"
   "static int\n"
   "_ostream_setCompression(_ostream *self, PyObject *value, void *closure)\n"
   "{\n"
   "   if (value == NULL) {\n"
   "      PyErr_SetString(PyExc_TypeError, \"unexpected null argument\");\n"
   "      return -1;\n"
   "   }\n"
   "   long flags = PyInt_AsLong(value);\n"
   "   if (flags == -1 && PyErr_Occurred()) {\n"
   "      return -1;\n"
   "   }\n"
   "   try {\n"
   "      self->ostr->setCompression(flags);\n"
   "   }\n"
   "   catch (std::exception& e) {\n"
   "      PyErr_SetString(PyExc_RuntimeError, e.what());\n"
   "      return -1;\n"
   "   }\n"
   "   return 0;\n"
   "}\n"
   "\n"
   "static PyObject*\n"
   "_ostream_getIntegrityChecks(_ostream *self, void *closure)\n"
   "{\n"
   "   PyObject *flags = Py_BuildValue(\"i\", self->ostr->getIntegrityChecks());\n"
   "   return flags;\n"
   "}\n"
   "\n"
   "static int\n"
   "_ostream_setIntegrityChecks(_ostream *self, PyObject *value, void *closure)\n"
   "{\n"
   "   if (value == NULL) {\n"
   "      PyErr_SetString(PyExc_TypeError, \"unexpected null argument\");\n"
   "      return -1;\n"
   "   }\n"
   "   long flags = PyInt_AsLong(value);\n"
   "   if (flags == -1 && PyErr_Occurred()) {\n"
   "      return -1;\n"
   "   }\n"
   "   try {\n"
   "      self->ostr->setIntegrityChecks(flags);\n"
   "   }\n"
   "   catch (std::exception& e) {\n"
   "      PyErr_SetString(PyExc_RuntimeError, e.what());\n"
   "      return -1;\n"
   "   }\n"
   "   return 0;\n"
   "}\n"
   "\n"
   "static PyObject*\n"
   "_ostream_getPosition(_ostream *self, void *closure)\n"
   "{\n"
   "   streamposition *pos = new streamposition();\n"
   "   if (self->ostr != 0)\n"
   "      *pos = self->ostr->getPosition();\n"
   "   PyObject *pos_obj = _streamposition_new(&_streamposition_type, 0, 0);\n"
   "   ((_streamposition*)pos_obj)->streampos = pos;\n"
   "   return pos_obj;\n"
   "}\n"
   "\n"
   "static PyObject*\n"
   "_ostream_getRecordsWritten(_ostream *self, void *closure)\n"
   "{\n"
   "   int records = 0;\n"
   "   if (self->ostr != 0)\n"
   "      try {\n"
   "         records = self->ostr->getRecordsWritten();\n"
   "      }\n"
   "      catch (std::exception& e) {\n"
   "         PyErr_SetString(PyExc_RuntimeError, e.what());\n"
   "         return NULL;\n"
   "      }\n"
   "   return PyLong_FromLong(records);\n"
   "}\n"
   "\n"
   "static PyObject*\n"
   "_ostream_getBytesWritten(_ostream *self, void *closure)\n"
   "{\n"
   "   int bytes = 0;\n"
   "   if (self->ostr != 0)\n"
   "      try {\n"
   "         bytes = self->ostr->getBytesWritten();\n"
   "      }\n"
   "      catch (std::exception& e) {\n"
   "         PyErr_SetString(PyExc_RuntimeError, e.what());\n"
   "         return NULL;\n"
   "      }\n"
   "   return PyLong_FromLong(bytes);\n"
   "}\n"
   "\n"
   "static PyObject*\n"
   "_ostream_write(PyObject *self, PyObject *args)\n"
   "{\n"
   "   _HDDM *record_obj;\n"
   "   if (! PyArg_ParseTuple(args, \"O!\", &_HDDM_type, (PyObject*)&record_obj))\n"
   "       return NULL;\n"
   "   ostream *ostr = ((_ostream*)self)->ostr;\n"
   "   try {\n"
   "      Py_BEGIN_ALLOW_THREADS\n"
   "      *ostr << *record_obj->elem;\n"
   "      Py_END_ALLOW_THREADS\n"
   "   }\n"
   "   catch (std::exception& e) {\n"
   "      PyErr_SetString(PyExc_RuntimeError, e.what());\n"
   "      return NULL;\n"
   "   }\n"
   "   Py_INCREF(Py_None);\n"
   "   return Py_None;\n"
   "}\n"
   "\n"
   "static PyObject*\n"
   "_ostream_toString(PyObject *self, PyObject *args=0)\n"
   "{\n"
   "   std::stringstream ostr;\n"
   "   if (((_ostream*)self)->fname != 0)\n"
   "      ostr << \"hddm_" << classPrefix << ".ostream(\\\"\"\n"
   "           << *((_ostream*)self)->fname << \"\\\")\";\n"
   "   else\n"
   "      ostr << \"hddm_" << classPrefix << ".ostream(NULL)\";\n"
   "   return PyUnicode_FromString(ostr.str().c_str());\n"
   "}\n"
   "\n"
   "static PyObject*\n"
   "_ostream_toRepr(PyObject *self, PyObject *args=0)\n"
   "{\n"
   "   std::stringstream ostr;\n"
   "   ostr << \"\\\'\";\n"
   "   if (((_ostream*)self)->fname != 0)\n"
   "      ostr << \"hddm_" << classPrefix << ".ostream(\\\"\"\n"
   "           << *((_ostream*)self)->fname << \"\\\")\";\n"
   "   else\n"
   "      ostr << \"hddm_" << classPrefix << ".ostream()\";\n"
   "   ostr << \"\\\'\";\n"
   "   return PyUnicode_FromString(ostr.str().c_str());\n"
   "}\n"
   "\n"
   "static PyGetSetDef _ostream_getsetters[] = {\n"
   "   {(char*)\"compression\", \n"
   "    (getter)_ostream_getCompression, (setter)_ostream_setCompression,\n"
   "    (char*)\"ostream compression mode (k_no_compression, k_z_compression, ...)\",\n"
   "    NULL},\n"
   "   {(char*)\"integrityChecks\", \n"
   "    (getter)_ostream_getIntegrityChecks, (setter)_ostream_setIntegrityChecks,\n"
   "    (char*)\"ostream data integrity checking mode (k_no_integrity, ...)\",\n"
   "    NULL},\n"
   "   {(char*)\"position\", \n"
   "    (getter)_ostream_getPosition, 0,\n"
   "    (char*)\"output stream position\",\n"
   "    NULL},\n"
   "   {(char*)\"recordsWritten\", \n"
   "    (getter)_ostream_getRecordsWritten, 0,\n"
   "    (char*)\"total records written to ostream\",\n"
   "    NULL},\n"
   "   {(char*)\"bytesWritten\", \n"
   "    (getter)_ostream_getBytesWritten, 0,\n"
   "    (char*)\"total bytes written to ostream\",\n"
   "    NULL},\n"
   "   {NULL}  /* Sentinel */\n"
   "};\n"
   "\n"
   "static PyMemberDef _ostream_members[] = {\n"
   "   {NULL}  /* Sentinel */\n"
   "};\n"
   "\n"
   "static PyMethodDef _ostream_methods[] = {\n"
   "   {\"write\",  _ostream_write, METH_VARARGS,\n"
   "    \"write a HDDM record to the output stream.\"},\n"
   "   {NULL}  /* Sentinel */\n"
   "};\n"
   "\n"
   "static PyTypeObject _ostream_type = {\n"
   "    PyVarObject_HEAD_INIT(NULL,0)\n"
   "    \"hddm_" << classPrefix << ".ostream\",          /*tp_name*/\n"
   "    sizeof(_ostream),          /*tp_basicsize*/\n"
   "    0,                         /*tp_itemsize*/\n"
   "    (destructor)_ostream_dealloc, /*tp_dealloc*/\n"
   "    0,                         /*tp_print*/\n"
   "    0,                         /*tp_getattr*/\n"
   "    0,                         /*tp_setattr*/\n"
   "    0,                         /*tp_compare*/\n"
   "    (reprfunc)_ostream_toRepr, /*tp_repr*/\n"
   "    0,                         /*tp_as_number*/\n"
   "    0,                         /*tp_as_sequence*/\n"
   "    0,                         /*tp_as_mapping*/\n"
   "    0,                         /*tp_hash */\n"
   "    0,                         /*tp_call*/\n"
   "    (reprfunc)_ostream_toString, /*tp_str*/\n"
   "    0,                         /*tp_getattro*/\n"
   "    0,                         /*tp_setattro*/\n"
   "    0,                         /*tp_as_buffer*/\n"
   "    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/\n"
   "    \"hddm_" << classPrefix << " output stream\",    /* tp_doc */\n"
   "    0,                         /* tp_traverse */\n"
   "    0,                         /* tp_clear */\n"
   "    0,                         /* tp_richcompare */\n"
   "    0,                         /* tp_weaklistoffset */\n"
   "    0,                         /* tp_iter */\n"
   "    0,                         /* tp_iternext */\n"
   "    _ostream_methods,          /* tp_methods */\n"
   "    _ostream_members,          /* tp_members */\n"
   "    _ostream_getsetters,       /* tp_getset */\n"
   "    0,                         /* tp_base */\n"
   "    0,                         /* tp_dict */\n"
   "    0,                         /* tp_descr_get */\n"
   "    0,                         /* tp_descr_set */\n"
   "    0,                         /* tp_dictoffset */\n"
   "    (initproc)_ostream_init,   /* tp_init */\n"
   "    0,                         /* tp_alloc */\n"
   "    _ostream_new,              /* tp_new */\n"
   "};\n"
   "\n"
   "\n"
   "// wrap class hddm_" << classPrefix << "::istream"
   " as hddm_" << classPrefix << ".istream\n"
   "\n"
   "typedef struct {\n"
   "   PyObject_HEAD\n"
   "   std::string *fname;\n"
   "   std::ifstream *fstr;\n"
   "   istream *istr;\n"
   "} _istream;\n"
   "\n"
   "static void\n"
   "_istream_dealloc(_istream* self)\n"
   "{\n"
   "   if (self->fname != 0)\n"
   "      delete self->fname;\n"
   "   if (self->istr != 0)\n"
   "      delete self->istr;\n"
   "   if (self->fstr != 0)\n"
   "      delete self->fstr;\n"
   "   Py_TYPE(self)->tp_free((PyObject*)self);\n"
   "}\n"
   "\n"
   "static PyObject*\n"
   "_istream_new(PyTypeObject *type, PyObject *args, PyObject *kwds)\n"
   "{\n"
   "   _istream *self;\n"
   "   self = (_istream*)type->tp_alloc(type, 0);\n"
   "   if (self != NULL) {\n"
   "      self->fname = 0;\n"
   "      self->fstr = 0;\n"
   "      self->istr = 0;\n"
   "   }\n"
   "   return (PyObject*)self;\n"
   "}\n"
   "\n"
   "static int\n"
   "_istream_init(_istream *self, PyObject *args, PyObject *kwds)\n"
   "{\n"
   "   const char *kwlist[] = {\"file\", NULL};\n"
   "   char *filename;\n"
   "   if (PyArg_ParseTupleAndKeywords(args, kwds, \"s\", (char**)kwlist, &filename))\n"
   "   {\n"
   "      if (self->fname != 0)\n"
   "         delete self->fname;\n"
   "      if (self->istr != 0)\n"
   "         delete self->istr;\n"
   "      if (self->fstr != 0)\n"
   "         delete self->fstr;\n"
   "      self->fname = new std::string(filename);\n"
   "      self->fstr = new std::ifstream(filename);\n"
   "      if (! self->fstr->good()) {\n"
   "         PyErr_Format(PyExc_IOError, \"Cannot open input file \\%s\", filename);\n"
   "         return -1;\n"
   "      }\n"
   "      try {\n"
   "         self->istr = new istream(*self->fstr);\n"
   "      }\n"
   "      catch (std::exception& e) {\n"
   "         PyErr_SetString(PyExc_RuntimeError, e.what());\n"
   "         return -1;\n"
   "      }\n"
   "      return 0;\n"
   "   }\n"
   "   return -1; \n"
   "}\n"
   "\n"
   "static PyObject*\n"
   "_istream_getCompression(_istream *self, void *closure)\n"
   "{\n"
   "   return Py_BuildValue(\"i\", self->istr->getCompression());\n"
   "}\n"
   "\n"
   "static PyObject*\n"
   "_istream_getIntegrityChecks(_istream *self, void *closure)\n"
   "{\n"
   "   return Py_BuildValue(\"i\", self->istr->getIntegrityChecks());\n"
   "}\n"
   "\n"
   "static PyObject*\n"
   "_istream_getPosition(_istream *self, void *closure)\n"
   "{\n"
   "   streamposition *pos = new streamposition();\n"
   "   if (self->istr != 0)\n"
   "      try {\n"
   "         *pos = self->istr->getPosition();\n"
   "      }\n"
   "      catch (std::exception& e) {\n"
   "         PyErr_SetString(PyExc_RuntimeError, e.what());\n"
   "         return NULL;\n"
   "      }\n"
   "   PyObject *pos_obj = _streamposition_new(&_streamposition_type, 0, 0);\n"
   "   ((_streamposition*)pos_obj)->streampos = pos;\n"
   "   return pos_obj;\n"
   "}\n"
   "\n"
   "static int\n"
   "_istream_setPosition(_istream *self, PyObject *value, void *closure)\n"
   "{\n"
   "   if (Py_TYPE(value) != &_streamposition_type)\n"
   "   {\n"
   "      PyErr_SetString(PyExc_TypeError, \"unexpected argument type\");\n"
   "      return -1;\n"
   "   }\n"
   "   streamposition *pos = ((_streamposition*)value)->streampos;\n"
   "   if (pos == 0) {\n"
   "      PyErr_SetString(PyExc_TypeError, \"unexpected null argument\");\n"
   "      return -1;\n"
   "   }\n"
   "   try {\n"
   "      self->istr->setPosition(*pos);\n"
   "   }\n"
   "   catch (std::exception& e) {\n"
   "      PyErr_SetString(PyExc_RuntimeError, e.what());\n"
   "      return -1;\n"
   "   }\n"
   "   return 0;\n"
   "}\n"
   "\n"
   "static PyObject*\n"
   "_istream_getRecordsRead(_istream *self, void *closure)\n"
   "{\n"
   "   int records = 0;\n"
   "   if (self->istr != 0)\n"
   "      try {\n"
   "         records = self->istr->getRecordsRead();\n"
   "      }\n"
   "      catch (std::exception& e) {\n"
   "         PyErr_SetString(PyExc_RuntimeError, e.what());\n"
   "         return NULL;\n"
   "      }\n"
   "   return PyLong_FromLong(records);\n"
   "}\n"
   "\n"
   "static PyObject*\n"
   "_istream_getBytesRead(_istream *self, void *closure)\n"
   "{\n"
   "   int bytes = 0;\n"
   "   if (self->istr != 0)\n"
   "      try {\n"
   "         bytes = self->istr->getBytesRead();\n"
   "      }\n"
   "      catch (std::exception& e) {\n"
   "         PyErr_SetString(PyExc_RuntimeError, e.what());\n"
   "         return NULL;\n"
   "      }\n"
   "   return PyLong_FromLong(bytes);\n"
   "}\n"
   "\n"
   "static PyObject*\n"
   "_istream_skip(PyObject *self, PyObject *args)\n"
   "{\n"
   "   int count=0;\n"
   "   if (! PyArg_ParseTuple(args, \"I\", &count)) {\n"
   "      PyErr_SetString(PyExc_TypeError, \"missing argument in skip\");\n"
   "      return NULL;\n"
   "   }\n"
   "   else if (count < 0) {\n"
   "      PyErr_SetString(PyExc_TypeError, \"skip count cannot be negative\");\n"
   "      return NULL;\n"
   "   }\n"
   "   istream *istr = ((_istream*)self)->istr;\n"
   "   if (istr == 0) {\n"
   "      PyErr_SetString(PyExc_TypeError, \"unexpected null istream ptr\");\n"
   "      return NULL;\n"
   "   }\n"
   "   istr->skip(count);\n"
   "   return PyLong_FromLong(0);\n"
   "}\n"
   "\n"
   "static PyObject*\n"
   "_istream_read(PyObject *self, PyObject *args)\n"
   "{\n"
   "   istream *istr = ((_istream*)self)->istr;\n"
   "   if (istr == 0) {\n"
   "      PyErr_SetString(PyExc_TypeError, \"unexpected null input stream\");\n"
   "      return NULL;\n"
   "   }\n"
   "   _HDDM *record_obj = (_HDDM*)_HDDM_new(&_HDDM_type, 0, 0);\n"
   "   record_obj->elem = new HDDM();\n"
   "   record_obj->host = (PyObject*)record_obj;\n"
   "   try {\n"
   "      Py_BEGIN_ALLOW_THREADS\n"
   "      *istr >> *record_obj->elem;\n"
   "      Py_END_ALLOW_THREADS\n"
   "   }\n"
   "   catch (std::exception& e) {\n"
   "      PyErr_SetString(PyExc_RuntimeError, e.what());\n"
   "      return NULL;\n"
   "   }\n"
   "   if (*istr) {\n"
   "      LOG_NEW(Py_TYPE(record_obj), 0, 1);\n"
   "      return (PyObject*)record_obj;\n"
   "   }\n"
   "   return NULL;\n"
   "}\n"
   "\n"
   "static PyObject*\n"
   "_istream_toString(PyObject *self, PyObject *args=0)\n"
   "{\n"
   "   std::stringstream ostr;\n"
   "   if (((_ostream*)self)->fname != 0)\n"
   "      ostr << \"hddm_" << classPrefix << ".istream(\\\"\"\n"
   "           << *((_istream*)self)->fname << \"\\\")\";\n"
   "   else\n"
   "      ostr << \"hddm_" << classPrefix << ".istream(NULL)\";\n"
   "   return PyUnicode_FromString(ostr.str().c_str());\n"
   "}\n"
   "\n"
   "static PyObject*\n"
   "_istream_toRepr(PyObject *self, PyObject *args=0)\n"
   "{\n"
   "   std::stringstream ostr;\n"
   "   ostr << \"\\\'\";\n"
   "   if (((_ostream*)self)->fname != 0)\n"
   "      ostr << \"hddm_" << classPrefix << ".istream(\\\"\"\n"
   "           << *((_istream*)self)->fname << \"\\\")\";\n"
   "   else\n"
   "      ostr << \"hddm_" << classPrefix << ".istream()\";\n"
   "   ostr << \"\\\'\";\n"
   "   return PyUnicode_FromString(ostr.str().c_str());\n"
   "}\n"
   "\n"
   "static PyObject*\n"
   "_istream_iter(PyObject *self)\n"
   "{\n"
   "   Py_INCREF(self);\n"
   "   return self;\n"
   "}\n"
   "static PyObject*\n"
   "_istream_next(PyObject *self)\n"
   "{\n"
   "   PyObject *rec = _istream_read(self, 0);\n"
   "   if (rec == NULL)\n"
   "      PyErr_SetString(PyExc_StopIteration, \"no more data on input stream\");\n"
   "   return rec;\n"
   "}\n"
   "\n"
   "static PyGetSetDef _istream_getsetters[] = {\n"
   "   {(char*)\"compression\", \n"
   "    (getter)_istream_getCompression, 0,\n"
   "    (char*)\"istream compression mode (k_no_compression, k_z_compression, ...)\",\n"
   "    NULL},\n"
   "   {(char*)\"integrityChecks\", \n"
   "    (getter)_istream_getIntegrityChecks, 0,\n"
   "    (char*)\"istream data integrity checking mode (k_no_integrity, ...)\",\n"
   "    NULL},\n"
   "   {(char*)\"position\", \n"
   "    (getter)_istream_getPosition, (setter)_istream_setPosition,\n"
   "    (char*)\"input stream position\",\n"
   "    NULL},\n"
   "   {(char*)\"recordsRead\", \n"
   "    (getter)_istream_getRecordsRead, 0,\n"
   "    (char*)\"total records read from istream\",\n"
   "    NULL},\n"
   "   {(char*)\"bytesRead\", \n"
   "    (getter)_istream_getBytesRead, 0,\n"
   "    (char*)\"total bytes read from istream\",\n"
   "    NULL},\n"
   "   {NULL}  /* Sentinel */\n"
   "};\n"
   "\n"
   "static PyMemberDef _istream_members[] = {\n"
   "   {NULL}  /* Sentinel */\n"
   "};\n"
   "\n"
   "static PyMethodDef _istream_methods[] = {\n"
   "   {\"read\",  _istream_read, METH_NOARGS,\n"
   "    \"read a HDDM record from the input stream.\"},\n"
   "   {\"skip\",  _istream_skip, METH_VARARGS,\n"
   "    \"skip ahead given number of HDDM records in the input stream.\"},\n"
   "   {NULL}  /* Sentinel */\n"
   "};\n"
   "\n"
   "static PyTypeObject _istream_type = {\n"
   "    PyVarObject_HEAD_INIT(NULL,0)\n"
   "    \"hddm_" << classPrefix << ".istream\",          /*tp_name*/\n"
   "    sizeof(_istream),          /*tp_basicsize*/\n"
   "    0,                         /*tp_itemsize*/\n"
   "    (destructor)_istream_dealloc, /*tp_dealloc*/\n"
   "    0,                         /*tp_print*/\n"
   "    0,                         /*tp_getattr*/\n"
   "    0,                         /*tp_setattr*/\n"
   "    0,                         /*tp_compare*/\n"
   "    (reprfunc)_istream_toRepr, /*tp_repr*/\n"
   "    0,                         /*tp_as_number*/\n"
   "    0,                         /*tp_as_sequence*/\n"
   "    0,                         /*tp_as_mapping*/\n"
   "    0,                         /*tp_hash */\n"
   "    0,                         /*tp_call*/\n"
   "    (reprfunc)_istream_toString, /*tp_str*/\n"
   "    0,                         /*tp_getattro*/\n"
   "    0,                         /*tp_setattro*/\n"
   "    0,                         /*tp_as_buffer*/\n"
   "    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/\n"
   "    \"hddm_" << classPrefix << " output stream\",    /* tp_doc */\n"
   "    0,                         /* tp_traverse */\n"
   "    0,                         /* tp_clear */\n"
   "    0,                         /* tp_richcompare */\n"
   "    0,                         /* tp_weaklistoffset */\n"
   "    _istream_iter,             /* tp_iter */\n"
   "    _istream_next,             /* tp_iternext */\n"
   "    _istream_methods,          /* tp_methods */\n"
   "    _istream_members,          /* tp_members */\n"
   "    _istream_getsetters,       /* tp_getset */\n"
   "    0,                         /* tp_base */\n"
   "    0,                         /* tp_dict */\n"
   "    0,                         /* tp_descr_get */\n"
   "    0,                         /* tp_descr_set */\n"
   "    0,                         /* tp_dictoffset */\n"
   "    (initproc)_istream_init,   /* tp_init */\n"
   "    0,                         /* tp_alloc */\n"
   "    _istream_new,              /* tp_new */\n"
   "};\n"
   ;

   builder.pyFile <<
   "\n"
   "\n"
   "// module declarations\n"
   "\n"
   "static PyMethodDef hddm_" << classPrefix << "_methods[] = {\n"
   "   {NULL}  /* Sentinel */\n"
   "};\n"
   "\n"
   "char hddm_" << classPrefix << "_doc[] = \"Python module for "
   "hddm_" << classPrefix << " i/o package\";\n"
   "\n"
   "#if PY_MAJOR_VERSION >= 3\n"
   "  static struct PyModuleDef moduledef = {\n"
   "    PyModuleDef_HEAD_INIT,\n"
   "    \"hddm_" << classPrefix << "\",            /* m_name */\n"
   "    hddm_" << classPrefix << "_doc,          /* m_doc */\n"
   "    -1,                  /* m_size */\n"
   "    hddm_" << classPrefix << "_methods,      /* m_methods */\n"
   "    NULL,                /* m_reload */\n"
   "    NULL,                /* m_traverse */\n"
   "    NULL,                /* m_clear */\n"
   "    NULL,                /* m_free */\n"
   "  };\n"
   "#endif\n"
   "\n"
   "static PyObject *\n"
   "hddm_" << classPrefix << "_init(void) \n"
   "{\n"
   "   PyObject* m;\n"
   "\n"
   "#if PY_MAJOR_VERSION >= 3\n"
   "   m = PyModule_Create(&moduledef);\n"
   "#else\n"
   "   m = Py_InitModule3(\"hddm_" << classPrefix << "\","
   " hddm_" << classPrefix << "_methods,"
   " hddm_" << classPrefix << "_doc);\n"
   "#endif\n"
   "\n"
   "   if (m == NULL)\n"
   "      return NULL;\n"
   "\n"
   ;

   std::map<XtString,XtString>::iterator titer;
   for (titer = builder.typesList.begin(); 
        titer != builder.typesList.end(); ++titer)
   {
      builder.pyFile <<
      "   if (PyType_Ready(&" << titer->second << ") < 0)\n"
      "      return NULL;\n"
      "   Py_INCREF(&" << titer->second << ");\n"
      "   PyModule_AddObject(m, \"" << titer->first << "\","
      " (PyObject*)&" << titer->second << ");\n"
      ;
   }

   builder.pyFile <<
   "\n"
   "   PyModule_AddIntConstant(m, \"k_default_status\", k_default_status);\n"
   "   PyModule_AddIntConstant(m, \"k_bits_compression\", k_bits_compression);\n"
   "   PyModule_AddIntConstant(m, \"k_no_compression\", k_no_compression);\n"
   "   PyModule_AddIntConstant(m, \"k_z_compression\", k_z_compression);\n"
   "   PyModule_AddIntConstant(m, \"k_bz2_compression\", k_bz2_compression);\n"
   "   PyModule_AddIntConstant(m, \"k_bits_integrity\", k_bits_integrity);\n"
   "   PyModule_AddIntConstant(m, \"k_no_integrity\", k_no_integrity);\n"
   "   PyModule_AddIntConstant(m, \"k_crc32_integrity\", k_crc32_integrity);\n"
   "   PyModule_AddIntConstant(m, \"k_bits_randomaccess\", k_bits_randomaccess);\n"
   "   PyModule_AddIntConstant(m, \"k_can_reposition\", k_can_reposition);\n"
   "   PyModule_AddIntConstant(m, \"k_hddm_unknown\", k_hddm_unknown);\n"
   "   PyModule_AddIntConstant(m, \"k_hddm_int\", k_hddm_int);\n"
   "   PyModule_AddIntConstant(m, \"k_hddm_long\", k_hddm_long);\n"
   "   PyModule_AddIntConstant(m, \"k_hddm_float\", k_hddm_float);\n"
   "   PyModule_AddIntConstant(m, \"k_hddm_double\", k_hddm_double);\n"
   "   PyModule_AddIntConstant(m, \"k_hddm_boolean\", k_hddm_boolean);\n"
   "   PyModule_AddIntConstant(m, \"k_hddm_string\", k_hddm_string);\n"
   "   PyModule_AddIntConstant(m, \"k_hddm_anyURI\", k_hddm_anyURI);\n"
   "   PyModule_AddIntConstant(m, \"k_hddm_Particle_t\", k_hddm_Particle_t);\n"
   "   std::string docstring = HDDM::DocumentString();\n"
   "   PyModule_AddStringConstant(m, \"DocumentString\", docstring.c_str());\n"
   "\n"
   "   return m;\n"
   "}\n"
   "\n"
   "#if PY_MAJOR_VERSION < 3\n"
   "   PyMODINIT_FUNC\n"
   "   inithddm_" << classPrefix << "(void)\n"
   "   {\n"
   "      hddm_" << classPrefix << "_init();\n"
   "   }\n"
   "#else\n"
   "   PyMODINIT_FUNC\n"
   "   PyInit_hddm_" << classPrefix << "(void)\n"
   "   {\n"
   "      return hddm_" << classPrefix << "_init();\n"
   "   }\n"
   "#endif\n"
   ;

/* convert cpy filename "<dirpath>/hddm_X.cpy"
 * to setup filename "<dirpath>/setup_hddm_X.py"
 */
   size_t p1 = pyname.rfind("pyhddm_");
   pyname.erase(p1, 2);
   pyname.insert(p1, "setup_");
   size_t p2 = pyname.rfind("cpy");
   pyname.erase(p2, 1);

   std::ofstream pysetup(pyname.c_str());
   pysetup <<
   "import os\n"
   "from distutils.core import setup, Extension\n"
   "from shutil import copyfile, rmtree\n"
   "import re\n"
   "\n"
   "source_dir = os.path.realpath(__file__)\n"
   "m = re.sub(r'/[^/]*$', '', source_dir)\n"
   "if m:\n"
   "   source_dir = m\n"
   "else:\n"
   "   source_dir = '.'\n"
   "source_file = 'pyhddm_" + classPrefix + ".cxx'\n"
   "source_files = [source_file, source_dir + '/hddm_" + classPrefix + "++.cpp']\n"
   "copyfile(source_dir + '/pyhddm_" + classPrefix + ".cpy', source_file)\n"
   "\n"
   "module1 = Extension('hddm_" + classPrefix + "',\n"
   "                    include_dirs = [os.environ['HALLD_RECON_HOME'] + '/' +\n"
   "                                    os.environ['BMS_OSNAME'] + '/include',\n"
   "                                    os.environ['HALLD_RECON_HOME'] + \n"
   "                                    '/src/libraries/include',\n"
   "                                    source_dir],\n"
   "                    libraries = ['HDDM', 'xstream', 'bz2', 'z'],\n"
   "                    library_dirs = [os.environ['HALLD_RECON_HOME'] + '/' +\n"
   "                                    os.environ['BMS_OSNAME'] + '/lib'],\n"
   "                    extra_compile_args = ['-std=c++11'],\n"
   "                    sources = source_files)\n"
   "\n"
   "setup (name = 'hddm_" << classPrefix << "',\n"
   "       version = '1.0',\n"
   "       description = 'HDDM data model i/o package',\n"
   "       ext_modules = [module1])\n"
   "\n"
   "os.remove(source_file)\n"
   "for dname in os.listdir('build_hddm_" << classPrefix << "'):\n"
   "    for soname in os.listdir('build_hddm_" << classPrefix << "/' + dname):\n"
   "        if re.match(r'.*\\.so', soname):\n"
   "            src = 'build_hddm_" << classPrefix << "/' + dname + '/' + soname\n"
   "            dest = source_dir + '/' + soname\n"
   "            copyfile(src, dest)\n"
   "rmtree('build_hddm_" + classPrefix << "')\n"
   ;

   XMLPlatformUtils::Terminate();
   return 0;
}

XtString XtString::plural()
{
   XtString p(*this);
   XtString::size_type len = p.size();
   if (len > 3 && p.substr(len-3,3) == "tum")
   {
      p.replace(len-3,3,"ta");
   }
   else if (len > 1 && p.substr(len-3,3) == "ies")
   {
      p.replace(len-3,3,"iesList");
   }
   else if (len > 2 && p.substr(len-2,2) == "ex")
   {
      p.replace(len-2,2,"ices");
   }
   else if (len > 2 && p.substr(len-2,2) == "sh")
   {
      p.replace(len-2,2,"shes");
   }
   else if (len > 1 && p.substr(len-1,1) == "s")
   {
      p.replace(len-1,1,"ses");
   }
   else if (len > 1)
   {
      p += "s";
   }
   return p;
}

/* Map from tag name to name of the corresponding class
 * for the case of simple tags (those that do not repeat)
 */
XtString XtString::simpleType()
{
   XtString p(*this);
   p[0] = toupper(p[0]);
   return p;
}

/* Map from tag name to name of the corresponding class
 * for the case of list tags (those that may repeat)
 */
XtString XtString::listType()
{
   XtString r(*this);
   r[0] = toupper(r[0]);
   r = r + "List";
   return r;
}

/* Map from tag name to name of the corresponding class
 * for the case of link tags (those that do not repeat)
 */
XtString XtString::linkType()
{
   XtString r(*this);
   r[0] = toupper(r[0]);
   r = r + "Link";
   return r;
}

/* Look for a named element in a list of element pointers
 * and return index of first instance in the list if found,
 * otherwise return -1;
 */
int CodeBuilder::element_in_list(XtString &name, parentList_t list)
{
   int n=0;
   parentList_t::iterator iter;
   for (iter = list.begin(); iter != list.end(); ++iter, ++n)
   {
      DOMElement *el = (DOMElement*)(*iter);
      XtString cnameS(el->getTagName());
      if (cnameS == name) {
         return n;
      }
   }
   return -1;
}

/* Verify that the tag group under this element does not collide
 * with existing tag group elref, otherwise exit with fatal error
 */
void CodeBuilder::checkConsistency(DOMElement* el, DOMElement* elref)
{
   XtString tagS(el->getTagName());
   if (el->getParentNode() == elref->getParentNode())
   {
      std::cerr
           << "hddm-py error: tag " << "\"" << tagS 
           << "\" is duplicated within one context in xml document."
       << std::endl;
      exit(1);
   }

   DOMNamedNodeMap* oldAttr = elref->getAttributes();
   DOMNamedNodeMap* newAttr = el->getAttributes();
   unsigned int listLength = oldAttr->getLength();
   for (unsigned int n = 0; n < listLength; n++)
   {
      XtString nameS(oldAttr->item(n)->getNodeName());
      XtString oldS(elref->getAttribute(X(nameS)));
      XtString newS(el->getAttribute(X(nameS)));
      if (nameS == "minOccurs")
      {
         continue;
      }
      else if (nameS == "maxOccurs")
      {
         int maxold = (oldS == "unbounded")? INT_MAX : atoi(S(oldS));
         int maxnew = (newS == "unbounded")? INT_MAX : atoi(S(newS));
     if ((maxold < 2 && maxnew > 1) || (maxold > 1 && maxnew < 2))
         {
            std::cerr
                 << "hddm-py error: inconsistent maxOccurs usage by tag "
                 << "\"" << tagS << "\" in xml document." << std::endl;
            exit(1);
         }
      }
      else if (newS != oldS)
      {
         std::cerr
              << "hddm-py error: inconsistent usage of attribute "
              << "\"" << nameS << "\" in tag "
              << "\"" << tagS << "\" in xml document." << std::endl;
         exit(1);
      }
   }
   listLength = newAttr->getLength();
   for (unsigned int n = 0; n < listLength; n++)
   {
      XtString nameS(newAttr->item(n)->getNodeName());
      XtString oldS(elref->getAttribute(X(nameS)));
      XtString newS(el->getAttribute(X(nameS)));
      if (nameS == "minOccurs")
      {
         continue;
      }
      else if (nameS == "maxOccurs")
      {
         int maxold = (oldS == "unbounded")? INT_MAX : atoi(S(oldS));
         int maxnew = (newS == "unbounded")? INT_MAX : atoi(S(newS));
     if ((maxold < 2 && maxnew > 1) || (maxold > 1 && maxnew < 2))
         {
            std::cerr
                 << "hddm-py error: inconsistent maxOccurs usage by tag "
                 << "\"" << tagS << "\" in xml document." << std::endl;
            exit(1);
         }
      }
      else if (newS != oldS)
      {
         std::cerr
              << "hddm-py error: inconsistent usage of attribute "
              << "\"" << nameS << "\" in tag "
              << "\"" << tagS << "\" in xml document." << std::endl;
         exit(1);
      }
   }
   DOMNodeList* oldList = elref->getChildNodes();
   DOMNodeList* newList = el->getChildNodes();
   listLength = oldList->getLength();
   if (newList->getLength() != listLength)
   {
      std::cerr
           << "hddm-py error: inconsistent usage of tag "
           << "\"" << tagS << "\" in xml document." << std::endl;
   exit(1);
   }
   for (unsigned int n = 0; n < listLength; n++)
   {
      DOMNode* cont = oldList->item(n);
      XtString nameS(cont->getNodeName());
      short type = cont->getNodeType();
      if (type == DOMNode::ELEMENT_NODE)
      {
         DOMNodeList* contList = el->getElementsByTagName(X(nameS));
         if (contList->getLength() != 1)
         {
             std::cerr
                  << "hddm-py error: inconsistent usage of tag "
                  << "\"" << tagS << "\" in xml document." << std::endl;
             exit(1);
         }
      }
   }
}

/* Write declaration of the classes for this tag */

void CodeBuilder::writeClassdef(DOMElement* el)
{
   XtString tagS(el->getTagName());
   pyFile << 
   "\n\n"
   "// wrap element class hddm_" << classPrefix << "::" << tagS.simpleType() <<
   " as hddm_" << classPrefix << "." << tagS.simpleType() << "\n"
   "\n"
   "typedef struct {\n"
   "   PyObject_HEAD\n"
   "   " << tagS.simpleType() << " *elem;\n"
   "   PyObject *host;\n"
   "} _" << tagS.simpleType() << ";\n"
   "\n"
   "static void\n"
   "_" << tagS.simpleType() << "_dealloc(_" << tagS.simpleType() << "* self)\n"
   "{\n"
   "   if (self->elem != 0) {\n"
   "      LOG_DEALLOC(Py_TYPE(self), 0, self->host == (PyObject*)self);\n"
   "      if (self->host == (PyObject*)self)\n"
   "         delete self->elem;\n"
   "      else\n"
   "         My_DECREF(self->host);\n"
   "   }\n"
   "   Py_TYPE(self)->tp_free((PyObject*)self);\n"
   "}\n"
   "\n"
   "static PyObject*\n"
   "_" << tagS.simpleType() <<
   "_new(PyTypeObject *type, PyObject *args, PyObject *kwds)\n"
   "{\n"
   "   _" << tagS.simpleType() << " *self;\n"
   "   self = (_" << tagS.simpleType() << "*)type->tp_alloc(type, 0);\n"
   "   if (self != NULL) {\n"
   "      self->elem = 0;\n"
   "      self->host = 0;\n"
   "   }\n"
   "   return (PyObject*)self;\n"
   "}\n"
   "\n"
   ;

   if (tagS == "HDDM")
   {
      pyFile << 
      "static int\n"
      "_HDDM_init(_HDDM *self, PyObject *args, PyObject *kwds)\n"
      "{\n"
      "   LOG_NEW(Py_TYPE(self), 0, 1);\n"
      "   self->elem = new HDDM();\n"
      "   if (self->elem == 0) {\n"
      "      PyErr_SetString(PyExc_RuntimeError, \"HDDM new constructor failed\");\n"
      "      return -1;\n"
      "   }\n"
      "   self->host = (PyObject*)self;\n"
      "   return 0;\n"
      "}\n"
      "\n"
      ;
   }
   else
   {
      pyFile << 
      "static int\n"
      "_" << tagS.simpleType() << "_init(_" << tagS.simpleType() << 
      " *self, PyObject *args, PyObject *kwds)\n"
      "{\n"
      "   PyErr_SetString(PyExc_RuntimeError, \"illegal constructor\");\n"
      "   return -1;\n"
      "}\n"
      "\n"
      ;
   }

   std::map<XtString,XtString> attrList;
   DOMNamedNodeMap *myAttr = el->getAttributes();
   for (unsigned int n = 0; n < myAttr->getLength(); n++)
   {
      XtString attrS(myAttr->item(n)->getNodeName());
      XtString typeS(el->getAttribute(X(attrS)));
      attrList[attrS] = typeS;
   }
   parentList_t::iterator iter;
   for (iter = parents[tagS].begin(); iter != parents[tagS].end(); ++iter)
   {
      DOMElement *hostEl = (DOMElement*)(*iter);
      XtString hostS(hostEl->getTagName());
      DOMNamedNodeMap *hostAttr = hostEl->getAttributes();
      for (unsigned int n = 0; n < hostAttr->getLength(); n++)
      {
         XtString attrS(hostAttr->item(n)->getNodeName());
         if (attrList.find(attrS) != attrList.end())
         {
            continue;
         }
         XtString typeS(hostEl->getAttribute(X(attrS)));
         attrList[attrS] = typeS;
         XtString getS("get" + attrS.simpleType());
         pyFile << "static PyObject*\n" <<
                   "_" << tagS.simpleType() << "_" << getS << 
                   "(_" << tagS.simpleType() << " *self, void *closure)\n"
                   "{\n";
         if (typeS == "int")
         {
            pyFile << "   return PyLong_FromLong(self->elem->" 
                   << getS << "());\n";
         }
         else if (typeS == "long")
         {
            pyFile << "   return PyLong_FromLong(self->elem->" 
                   << getS << "());\n";
         }
         else if (typeS == "float")
         {
            pyFile << "   return PyFloat_FromDouble(self->elem->" 
                   << getS << "());\n";
         }
         else if (typeS == "double")
         {
            pyFile << "   return PyFloat_FromDouble(self->elem->" 
                   << getS << "());\n";
         }
         else if (typeS == "boolean")
         {
            pyFile << "   return PyBool_FromLong(self->elem->" 
                   << getS << "());\n";
         }
         else if (typeS == "string")
         {
            pyFile << "   std::string val(self->elem->" 
                   << getS << "());\n"
                   << "   return PyUnicode_FromString(val.c_str());\n";
            attrList[attrS] = "string";
         }
         else if (typeS == "anyURI")
         {
            pyFile << "   std::string val(self->elem->" 
                   << getS << "());\n"
                   << "   return PyUnicode_FromString(val.c_str());\n";
            attrList[attrS] = "string";
         }
         else if (typeS == "Particle_t")
         {
            pyFile << "   Particle_t p(self->elem->"
                   << getS << "());\n"
                   << "   std::string val(ParticleType(p));\n"
                   << "   return PyUnicode_FromString(val.c_str());\n";
         }
         else if (guessType(typeS) == "int")
         {
            pyFile << "   return PyLong_FromLong(self->elem->" 
                   << getS << "());\n";
         }
         else if (guessType(typeS) == "long")
         {
            pyFile << "   return PyLong_FromLong(self->elem->" 
                   << getS << "());\n";
         }
         else if (guessType(typeS) == "float")
         {
            pyFile << "   return PyFloat_FromDouble(self->elem->" 
                   << getS << "());\n";
         }
         else if (guessType(typeS) == "double")
         {
            pyFile << "   return PyFloat_FromDouble(self->elem->" 
                   << getS << "());\n";
         }
         else if (guessType(typeS) == "boolean")
         {
            pyFile << "   return PyBool_FromLong(self->elem->" 
                   << getS << "());\n";
         }
         else if (guessType(typeS) == "Particle_t")
         {
            pyFile << "   Particle_t p(self->elem->"
                   << getS << "());\n"
                   << "   std::string val(ParticleType(p));\n"
                   << "   return PyUnicode_FromString(val.c_str());\n";
         }
         else /* any values not matching the above are strings */
         {
            pyFile << "   std::string val(self->elem->" 
                   << getS << "());\n"
                   << "   return PyUnicode_FromString(val.c_str());\n";
            attrList[attrS] = "string";
         }
         pyFile << "}\n\n";
      }
   }

   std::map<XtString,int> setters;
   myAttr = el->getAttributes();
   for (unsigned int n = 0; n < myAttr->getLength(); n++)
   {
      XtString attrS(myAttr->item(n)->getNodeName());
      XtString typeS(el->getAttribute(X(attrS)));
      XtString getS("get" + attrS.simpleType());
      pyFile << "static PyObject*\n" <<
                "_" << tagS.simpleType() << "_" << getS << 
                "(_" << tagS.simpleType() << " *self, void *closure)\n"
                "{\n";
      if (typeS == "int")
      {
         pyFile << "   return PyLong_FromLong(self->elem->" 
                << getS << "());\n";
      }
      else if (typeS == "long")
      {
         pyFile << "   return PyLong_FromLong(self->elem->" 
                << getS << "());\n";
      }
      else if (typeS == "float")
      {
         pyFile << "   return PyFloat_FromDouble(self->elem->" 
                << getS << "());\n";
      }
      else if (typeS == "double")
      {
         pyFile << "   return PyFloat_FromDouble(self->elem->" 
                << getS << "());\n";
      }
      else if (typeS == "boolean")
      {
         pyFile << "   return PyBool_FromLong(self->elem->" 
                << getS << "());\n";
      }
      else if (typeS == "string")
      {
         pyFile << "   std::string val(self->elem->" 
                << getS << "());\n"
                << "   return PyUnicode_FromString(val.c_str());\n";
      }
      else if (typeS == "anyURI")
      {
         pyFile << "   std::string val(self->elem->" 
                << getS << "());\n"
                << "   return PyUnicode_FromString(val.c_str());\n";
      }
      else if (typeS == "Particle_t")
      {
         pyFile << "   Particle_t p(self->elem->"
                << getS << "());\n"
                << "   std::string val(ParticleType(p));\n"
                << "   return PyUnicode_FromString(val.c_str());\n";
      }
      else if (guessType(typeS) == "int")
      {
         pyFile << "   return PyLong_FromLong(self->elem->" 
                << getS << "());\n";
      }
      else if (guessType(typeS) == "long")
      {
         pyFile << "   return PyLong_FromLong(self->elem->" 
                << getS << "());\n";
      }
      else if (guessType(typeS) == "float")
      {
         pyFile << "   return PyFloat_FromDouble(self->elem->" 
                << getS << "());\n";
      }
      else if (guessType(typeS) == "double")
      {
         pyFile << "   return PyFloat_FromDouble(self->elem->" 
                << getS << "());\n";
      }
      else if (guessType(typeS) == "boolean")
      {
         pyFile << "   return PyBool_FromLong(self->elem->" 
                << getS << "());\n";
      }
      else if (guessType(typeS) == "Particle_t")
      {
         pyFile << "   Particle_t p(self->elem->"
                << getS << "());\n"
                << "   std::string val(ParticleType(p));\n"
                << "   return PyUnicode_FromString(val.c_str());\n";
      }
      else /* any values not matching the above are strings */
      {
         pyFile << "   std::string val(self->elem->" 
                << getS << "());\n"
                << "   return PyUnicode_FromString(val.c_str());\n";
      }
      pyFile << "}\n\n";

      XtString setS("set" + attrS.simpleType());
      if (typeS == "int")
      {
         pyFile << "static int\n" <<
                   "_" << tagS.simpleType() << "_" << setS << 
                   "(_" << tagS.simpleType() << 
                   " *self, PyObject *value, void *closure)\n"
                   "{\n"
                   "   long var = PyInt_AsLong(value);\n"
                   "   if (var == -1 && PyErr_Occurred()) {\n"
                   "      return -1;\n"
                   "   }\n"
                   "   self->elem->" << setS << "(var);\n"
                   "   return 0;\n"
                   "}\n\n";
         setters[attrS] = 1;
      }
      else if (typeS == "long")
      {
         pyFile << "static int\n" <<
                   "_" << tagS.simpleType() << "_" << setS << 
                   "(_" << tagS.simpleType() << 
                   " *self, PyObject *value, void *closure)\n"
                   "{\n"
                   "   long var = PyInt_AsLong(value);\n"
                   "   if (var == -1 && PyErr_Occurred()) {\n"
                   "      return -1;\n"
                   "   }\n"
                   "   self->elem->" << setS << "(var);\n"
                   "   return 0;\n"
                   "}\n\n";
         setters[attrS] = 1;
      }
      else if (typeS == "float")
      {
         pyFile << "static int\n" <<
                   "_" << tagS.simpleType() << "_" << setS << 
                   "(_" << tagS.simpleType() << 
                   " *self, PyObject *value, void *closure)\n"
                   "{\n"
                   "   double var = PyFloat_AsDouble(value);\n"
                   "   if (var == -1 && PyErr_Occurred()) {\n"
                   "      return 1;\n"
                   "   }\n"
                   "   self->elem->" << setS << "(var);\n"
                   "   return 0;\n"
                   "}\n\n";
         setters[attrS] = 1;
      }
      else if (typeS == "double")
      {
         pyFile << "static int\n" <<
                   "_" << tagS.simpleType() << "_" << setS << 
                   "(_" << tagS.simpleType() << 
                   " *self, PyObject *value, void *closure)\n"
                   "{\n"
                   "   double var = PyFloat_AsDouble(value);\n"
                   "   if (var == -1 && PyErr_Occurred()) {\n"
                   "      return -1;\n"
                   "   }\n"
                   "   self->elem->" << setS << "(var);\n"
                   "   return 0;\n"
                   "}\n\n";
         setters[attrS] = 1;
      }
      else if (typeS == "boolean")
      {
         pyFile << "static int\n" <<
                   "_" << tagS.simpleType() << "_" << setS << 
                   "(_" << tagS.simpleType() << 
                   " *self, PyObject *value, void *closure)\n"
                   "{\n"
                   "   long var = PyInt_AsLong(value);\n"
                   "   if (var == -1 && PyErr_Occurred()) {\n"
                   "      return -1;\n"
                   "   }\n"
                   "   self->elem->" << setS << "((var==0)? false : true);\n"
                   "   return 0;\n"
                   "}\n\n";
         setters[attrS] = 1;
      }
      else if (typeS == "string")
      {
         pyFile << "static int\n" <<
                   "_" << tagS.simpleType() << "_" << setS << 
                   "(_" << tagS.simpleType() << 
                   " *self, PyObject *value, void *closure)\n"
                   "{\n"
                   "   PyObject *str=0;\n"
                   "   if (PyUnicode_Check(value))\n"
                   "      str = PyUnicode_AsEncodedString(value, \"ASCII\", \"strict\");\n"
                   "   else\n"
                   "      str = value;\n"
                   "#if PY_MAJOR_VERSION < 3\n"
                   "   char *var = PyString_AsString(str);\n"
                   "#else\n"
                   "   char *var = PyBytes_AsString(str);\n"
                   "#endif\n"
                   "   if (var == 0) {\n"
                   "      return -1;\n"
                   "   }\n"
                   "   self->elem->" << setS << "(std::string(var));\n"
                   "   if (str != value) {\n"
                   "      Py_DECREF(str);\n"
                   "   }\n"
                   "   return 0;\n"
                   "}\n\n";
         setters[attrS] = 1;
      }
      else if (typeS == "anyURI")
      {
         pyFile << "static int\n" <<
                   "_" << tagS.simpleType() << "_" << setS << 
                   "(_" << tagS.simpleType() << 
                   " *self, PyObject *value, void *closure)\n"
                   "{\n"
                   "   PyObject *str=0;\n"
                   "   if (PyUnicode_Check(value))\n"
                   "      str = PyUnicode_AsEncodedString(value, \"ASCII\", \"strict\");\n"
                   "   else\n"
                   "      str = value;\n"
                   "#if PY_MAJOR_VERSION < 3\n"
                   "   char *var = PyString_AsString(str);\n"
                   "#else\n"
                   "   char *var = PyBytes_AsString(str);\n"
                   "#endif\n"
                   "   if (var == 0) {\n"
                   "      return -1;\n"
                   "   }\n"
                   "   self->elem->" << setS << "(std::string(var));\n"
                   "   if (str != value) {\n"
                   "      Py_DECREF(str);\n"
                   "   }\n"
                   "   return 0;\n"
                   "}\n\n";
         setters[attrS] = 1;
      }
      else if (typeS == "Particle_t")
      {
         pyFile << "static int\n" <<
                   "_" << tagS.simpleType() << "_" << setS << 
                   "(_" << tagS.simpleType() << 
                   " *self, PyObject *value, void *closure)\n"
                   "{\n"
                   "   long var = PyInt_AsLong(value);\n"
                   "   if (var == -1 && PyErr_Occurred()) {\n"
                   "      return -1;\n"
                   "   }\n"
                   "   self->elem->" << setS << "((Particle_t)var);\n"
                   "   return 0;\n"
                   "}\n\n";
         setters[attrS] = 1;
      }
   }

   std::map<XtString,method_descr> methods;

   if (tagS == "HDDM") {
      parentTable_t::iterator piter;
      for (piter = parents.begin(); piter != parents.end(); ++piter)
      {
         XtString cnameS(piter->first);
         if (cnameS != "HDDM" && element_in_list(cnameS,children[tagS]) == -1)
         {
            XtString getS("_" + tagS.simpleType() + "_get" 
                              + cnameS.plural().simpleType());
            pyFile << "static PyObject*\n" << getS <<
                      "(PyObject *self, PyObject *args)\n"
                      "{\n"
                      "   _" << tagS.simpleType() << 
                      " *me = (_" << tagS.simpleType() << "*)self;\n"
                      "   if (me->elem == 0) {\n"
                      "      PyErr_SetString(PyExc_RuntimeError, "
                      "\"lookup attempted on invalid " << tagS << 
                      " element\");\n"
                      "      return NULL;\n"
                      "   }\n"
                      "   PyObject *list = _HDDM_ElementList"
                      "_new(&_HDDM_ElementList_type, 0, 0);\n"
                      "   ((_HDDM_ElementList*)list)->subtype = "
                      "&_" << cnameS.simpleType() << "_type;\n"
                      "   ((_HDDM_ElementList*)list)->list = "
                      "(HDDM_ElementList<HDDM_Element>*)\n" << "    "
                      "new " << cnameS.listType() << "("
                      "me->elem->get" << cnameS.plural().simpleType() << "());\n"
                      "   ((_HDDM_ElementList*)list)->borrowed = 0;\n"
                      "   ((_HDDM_ElementList*)list)->host = me->host;\n"
                      "   My_INCREF(me->host);\n"
                      "   LOG_NEW(Py_TYPE(list), "
                      "((_HDDM_ElementList*)list)->subtype, 1);\n"
                      "   return list;\n"
                      "}\n\n"
                      ;
            method_descr meth_getS = {"get" + cnameS.plural().simpleType(), 
                                      "METH_NOARGS", 
                                      "get complete list of " + cnameS +
                                      " objects for this record"};
            methods[getS] = meth_getS;
         }
      }
   }

   parentList_t::iterator citer;
   for (citer = children[tagS].begin(); citer != children[tagS].end(); ++citer)
   {
      DOMElement *childEl = (DOMElement*)(*citer);
      XtString cnameS(childEl->getTagName());
      XtString repS(childEl->getAttribute(X("maxOccurs")));
      int rep = (repS == "unbounded")? INT_MAX : atoi(S(repS));
      XtString getS("_" + tagS.simpleType() + "_get" + cnameS.simpleType());
      pyFile << "static PyObject*\n" << getS <<
                "(PyObject *self, PyObject *args)\n"
                "{\n"
                "   int index=0;\n"
                "   if (! PyArg_ParseTuple(args, \"|i\", &index)) {\n"
                "      return NULL;\n"
                "   }\n"
                "   _" << tagS.simpleType() << 
                " *me = (_" << tagS.simpleType() << "*)self;\n"
                "   if (me->elem == 0) {\n"
                "      PyErr_SetString(PyExc_RuntimeError, "
                "\"lookup attempted on invalid " << tagS << 
                " element\");\n"
                "      return NULL;\n"
                "   }\n"
                "   PyObject *obj = _" << cnameS.simpleType() <<
                "_new(&_" << cnameS.simpleType() << "_type, 0, 0);\n"
                "   ((_" << cnameS.simpleType() << 
                "*)obj)->elem = &me->elem->get" << cnameS.simpleType() 
                << ((rep > 1)? "(index)" : "()") << ";\n"
                "   ((_" << cnameS.simpleType() << "*)obj)->host = me->host;\n"
                "   My_INCREF(me->host);\n"
                "   LOG_NEW(Py_TYPE(obj));\n"
                "   return obj;\n"
                "}\n\n"
                ;
      method_descr meth_getS = {"get" + cnameS.simpleType(),
                                "METH_VARARGS",
                                "get an individual " + cnameS + 
                                " object from this " + tagS};
      methods[getS] = meth_getS;

      XtString gelS("_" + tagS.simpleType()
                        + "_get" + cnameS.plural().simpleType());
      pyFile << "static PyObject*\n" << gelS <<
                "(PyObject *self, PyObject *args)\n"
                "{\n"
                "   _" << tagS.simpleType() << 
                " *me = (_" << tagS.simpleType() << "*)self;\n"
                "   if (me->elem == 0) {\n"
                "      PyErr_SetString(PyExc_RuntimeError, "
                "\"lookup attempted on invalid " << tagS << 
                " element\");\n"
                "      return NULL;\n"
                "   }\n"
                "   PyObject *list = _HDDM_ElementList_new"
                "(&_HDDM_ElementList_type, 0, 0);\n"
                "   ((_HDDM_ElementList*)list)->subtype =" 
                " &_" << cnameS.simpleType() << "_type;\n"
                "   ((_HDDM_ElementList*)list)->list = " 
                "(HDDM_ElementList<HDDM_Element>*)\n" << "    "
                "&me->elem->get" << cnameS.plural().simpleType() << "();\n"
                "   ((_HDDM_ElementList*)list)->borrowed = 1;\n"
                "   ((_HDDM_ElementList*)list)->host = me->host;\n"
                "   My_INCREF(me->host);\n"
                "   LOG_NEW(Py_TYPE(list), "
                "((_HDDM_ElementList*)list)->subtype, 0);\n" 
                "   return list;\n"
                "}\n\n"
                ;
      method_descr meth_gelS = {"get" + cnameS.plural().simpleType(),
                                "METH_NOARGS",
                                "get list of " + cnameS +
                                " objects for this " + tagS};
      methods[gelS] = meth_gelS;

      XtString addS("_" + tagS.simpleType() 
                        + "_add" + cnameS.plural().simpleType());
      pyFile << "static PyObject*\n" << addS <<
                "(PyObject *self, PyObject *args)\n"
                "{\n"
                "   int count=1;\n"
                "   int start=-1;\n"
                "   if (! PyArg_ParseTuple(args, \"|ii\", &count, &start)) {\n"
                "      return NULL;\n"
                "   }\n;"
                "   _" << tagS.simpleType() << 
                " *me = (_" << tagS.simpleType() << "*)self;\n"
                "   if (me->elem == 0) {\n"
                "      PyErr_SetString(PyExc_RuntimeError, "
                "\"add attempted on invalid " << tagS << 
                " element\");\n"
                "      return NULL;\n"
                "   }\n"
                "   PyObject *list = _HDDM_ElementList_new"
                "(&_HDDM_ElementList_type, 0, 0);\n"
                "   ((_HDDM_ElementList*)list)->subtype =" 
                " &_" << cnameS.simpleType() << "_type;\n"
                "   ((_HDDM_ElementList*)list)->list = " 
                "(HDDM_ElementList<HDDM_Element>*)\n" << "    "
                "new " << cnameS.listType() << "("
                "me->elem->add" << cnameS.plural().simpleType() << 
                "(count, start));\n"
                "   ((_HDDM_ElementList*)list)->borrowed = 0;\n"
                "   ((_HDDM_ElementList*)list)->host = me->host;\n"
                "   My_INCREF(me->host);\n"
                "   LOG_NEW(Py_TYPE(list), "
                "((_HDDM_ElementList*)list)->subtype, 1);\n" 
                "   return list;\n"
                "}\n\n"
                ;
      method_descr meth_addS = {"add" + cnameS.plural().simpleType(),
                                "METH_VARARGS",
                                "extend (or insert into) the list of " + cnameS +
                                " objects for this " + tagS};
      methods[addS] = meth_addS;

      XtString delS("_" + tagS.simpleType() 
                        + "_delete" + cnameS.plural().simpleType());
      pyFile << "static PyObject*\n" << delS <<
                "(PyObject *self, PyObject *args)\n"
                "{\n"
                "   int count=-1;\n"
                "   int start=0;\n"
                "   if (! PyArg_ParseTuple(args, \"|ii\", &count, &start)) {\n"
                "      return NULL;\n"
                "   }\n;"
                "   _" << tagS.simpleType() << 
                " *me = (_" << tagS.simpleType() << "*)self;\n"
                "   if (me->elem == 0) {\n"
                "      PyErr_SetString(PyExc_RuntimeError, "
                "\"delete attempted on invalid " << tagS << 
                " element\");\n"
                "      return NULL;\n"
                "   }\n"
                "   me->elem->delete" << cnameS.plural().simpleType() << 
                "(count, start);\n"
                "   Py_INCREF(Py_None);\n"
                "   return Py_None;\n"
                "}\n\n"
                ;
      method_descr meth_delS = {"delete" + cnameS.plural().simpleType(),
                                "METH_VARARGS",
                                "delete " + cnameS + " objects for this " + tagS};
      methods[delS] = meth_delS;
   }

   if (tagS == "HDDM")
   {
      XtString clrS("_" + tagS.simpleType() + "_clear");
      pyFile << "static PyObject*\n" << clrS <<
                "(PyObject *self, PyObject *args)\n"
                "{\n"
                "   _" << tagS.simpleType() <<
                " *me = (_" << tagS.simpleType() << "*)self;\n"
                "   if (me->elem == 0) {\n"
                "      PyErr_SetString(PyExc_RuntimeError, "
                "\"lookup attempted on invalid " << tagS << 
                " element\");\n"
                "      return NULL;\n"
                "   }\n"
                "   me->elem->clear();\n"
                "   Py_INCREF(Py_None);\n"
                "   return Py_None;\n"
                "}\n\n"
                ;
      method_descr meth_clrS = {"clear", "METH_NOARGS",
                                "clear all contents from this " + tagS};
      methods[clrS] = meth_clrS;
   }

   XtString strS("_" + tagS.simpleType() + "_toString");
   pyFile << "static PyObject*\n" << strS <<
             "(PyObject *self, PyObject *args=0)\n"
             "{\n"
             "   _" << tagS.simpleType() <<
             " *me = (_" << tagS.simpleType() << "*)self;\n"
             "   if (me->elem == 0) {\n"
             "      PyErr_SetString(PyExc_RuntimeError, "
             "\"lookup attempted on invalid " << tagS << 
             " element\");\n"
             "      return NULL;\n"
             "   }\n"
             "   std::string str(me->elem->toString());\n"
             "   return PyUnicode_FromString(str.c_str());\n"
             "}\n\n"
             ;
   method_descr str_method = {"toString", "METH_NOARGS",
                              "show element as a human-readable string"};
   methods[strS] = str_method;

   XtString xmlS("_" + tagS.simpleType() + "_toXML");
   pyFile << "static PyObject*\n" << xmlS <<
             "(PyObject *self, PyObject *args=0)\n"
             "{\n"
             "   _" << tagS.simpleType() <<
             " *me = (_" << tagS.simpleType() << "*)self;\n"
             "   if (me->elem == 0) {\n"
             "      PyErr_SetString(PyExc_RuntimeError, "
             "\"lookup attempted on invalid " << tagS << 
             " element\");\n"
             "      return NULL;\n"
             "   }\n"
             "   std::string str(me->elem->toXML());\n"
             "   return PyUnicode_FromString(str.c_str());\n"
             "}\n\n"
             ;
   method_descr xml_method = {"toXML", "METH_NOARGS",
                              "show element as a XML fragment"};
   methods[xmlS] = xml_method;

   pyFile << "static PyGetSetDef _" << tagS.simpleType() 
          << "_getsetters[] = {\n";
   std::map<XtString,XtString>::iterator aiter;
   for (aiter = attrList.begin(); aiter != attrList.end(); ++aiter) {
      XtString attrS = aiter->first;
      XtString getterS("_" + tagS.simpleType() + "_" + "get" + attrS.simpleType());
      XtString setterS("_" + tagS.simpleType() + "_" + "set" + attrS.simpleType());
      pyFile << "   {(char*)\"" << attrS << "\",\n"
             << "    (getter)" << getterS << ", ";
      if (setters.find(attrS) != setters.end()) {
         pyFile << "(setter)" << setterS << ",\n";
      }
      else {
         pyFile << "0,\n";
      }
      if (aiter->second == "string") {
         pyFile << "    (char*)\"" << attrS << " string\",\n";
      }
      else {
         pyFile << "    (char*)\"" << attrS << " value\",\n";
      }
      pyFile << "    NULL},\n";
   }
   pyFile << "   {NULL}  /* Sentinel */\n"
             "};\n\n";

   pyFile << "static PyMemberDef _" << tagS.simpleType() 
          << "_members[] = {\n"
          << "   {NULL}  /* Sentinel */\n"
          << "};\n\n";

   pyFile << "static PyMethodDef _" << tagS.simpleType()
          << "_methods[] = {\n";
   std::map<XtString,method_descr>::iterator miter;
   for (miter = methods.begin(); miter != methods.end(); ++miter) {
      pyFile << "   {\"" << miter->second.name << "\", "
             << miter->first << ", " << miter->second.args << ",\n"
             << "    \"" << miter->second.docs << "\"},\n";
   }
   pyFile << "   {NULL}  /* Sentinel */\n"
          << "};\n\n";

   typesList[tagS] = "_" + tagS.simpleType() + "_type";

   pyFile <<
   "static PyTypeObject _" << tagS.simpleType() << "_type = {\n"
   "    PyVarObject_HEAD_INIT(NULL,0)\n"
   "    \"hddm_" << classPrefix << "." << tagS.simpleType() << "\","
   "         /*tp_name*/\n"
   "    sizeof(_" << tagS.simpleType() << 
   "),          /*tp_basicsize*/\n"
   "    0,                         /*tp_itemsize*/\n"
   "    (destructor)_" << tagS.simpleType() << 
   "_dealloc, /*tp_dealloc*/\n"
   "    0,                         /*tp_print*/\n"
   "    0,                         /*tp_getattr*/\n"
   "    0,                         /*tp_setattr*/\n"
   "    0,                         /*tp_compare*/\n"
   "    0,                         /*tp_repr*/\n"
   "    0,                         /*tp_as_number*/\n"
   "    0,                         /*tp_as_sequence*/\n"
   "    0,                         /*tp_as_mapping*/\n"
   "    0,                         /*tp_hash */\n"
   "    0,                         /*tp_call*/\n"
   "    (reprfunc)_" << tagS.simpleType() << "_toString,         /*tp_str*/\n"
   "    0,                         /*tp_getattro*/\n"
   "    0,                         /*tp_setattro*/\n"
   "    0,                         /*tp_as_buffer*/\n"
   "    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/\n"
   "    \"hddm_" << classPrefix << " " << tagS << 
   " element\",  /* tp_doc */\n"
   "    0,                         /* tp_traverse */\n"
   "    0,                         /* tp_clear */\n"
   "    0,                         /* tp_richcompare */\n"
   "    0,                         /* tp_weaklistoffset */\n"
   "    0,                         /* tp_iter */\n"
   "    0,                         /* tp_iternext */\n"
   "    _" << tagS.simpleType() << "_methods,          /* tp_methods */\n"
   "    _" << tagS.simpleType() << "_members,          /* tp_members */\n"
   "    _" << tagS.simpleType() << "_getsetters,       /* tp_getset */\n"
   "    &_HDDM_Element_type,       /* tp_base */\n"
   "    0,                         /* tp_dict */\n"
   "    0,                         /* tp_descr_get */\n"
   "    0,                         /* tp_descr_set */\n"
   "    0,                         /* tp_dictoffset */\n"
   "    (initproc)_" << tagS.simpleType() << "_init,   /* tp_init */\n"
   "    0,                         /* tp_alloc */\n"
   "    _" << tagS.simpleType() << "_new,              /* tp_new */\n"
   "};\n\n"
   ;
}

/* Generate class declarations for this tag and its descendants;
 * this function calls itself recursively
 */

void CodeBuilder::constructGroup(DOMElement* el)
{
   XtString tagS(el->getTagName());
   parentList_t::iterator piter;
   parents[tagS].insert(parents[tagS].begin(),
                        parentList.begin(),parentList.end());
   std::vector<DOMElement*>::iterator iter;
   for (iter = tagList.begin(); iter != tagList.end(); iter++)
   {
      DOMElement* targEl = *iter;
      XtString targS(targEl->getTagName());
      if (tagS == targS)
      {
         checkConsistency(el,targEl);
         return;
      }
   }

   parentList.push_back(el);
   DOMNodeList* contList = el->getChildNodes();
   int contLength = contList->getLength();
   for (int c = 0; c < contLength; c++)
   {
      DOMNode* cont = contList->item(c);
      short type = cont->getNodeType();
      if (type == DOMNode::ELEMENT_NODE)
      {
         DOMElement* contEl = (DOMElement*) cont;
         XtString contS(contEl->getTagName());
         children[tagS].push_back(contEl);
         constructGroup(contEl);
      }
   }
   parentList.pop_back();

   tagList.push_back(el);

   if (tagS == "HDDM")
   {
      std::vector<DOMElement*>::iterator iter;
      for (iter = tagList.begin(); iter != tagList.end(); iter++)
      {
         writeClassdef(*iter);
      }
   }
}

/* Write method implementation of the classes for this tag */

void CodeBuilder::writeClassimp(DOMElement* el)
{
}

/* Generate implementation code for data model classes */

void CodeBuilder::constructMethods(DOMElement* el)
{
   std::vector<DOMElement*>::iterator iter;
   for (iter = tagList.begin(); iter != tagList.end(); iter++)
   {
      writeClassimp(*iter);
   }
}

/* Generate methods for serializing classes to a stream and back again */

void CodeBuilder::writeStreamers(DOMElement* el)
{
}

void CodeBuilder::constructStreamers(DOMElement* el)
{
   std::vector<DOMElement*>::iterator iter;
   for (iter = tagList.begin(); iter != tagList.end(); ++iter)
   {
      writeStreamers(*iter);
   }
}
 
/* Generate methods to read from binary stream into classes */

void CodeBuilder::constructIOstreams(DOMElement* el)
{
}

/* Generate the xml template in normal form and store in a string */

void CodeBuilder::constructDocument(DOMElement* el)
{
   static int indent = 0;
   pyFile << "\"";
   for (int n = 0; n < indent; n++)
   {
      pyFile << "  ";
   }
   
   XtString tagS(el->getTagName());
   pyFile << "<" << tagS;
   DOMNamedNodeMap* attrList = el->getAttributes();
   int attrListLength = attrList->getLength();
   for (int a = 0; a < attrListLength; a++)
   {
      DOMNode* node = attrList->item(a);
      XtString nameS(node->getNodeName());
      XtString valueS(node->getNodeValue());
      pyFile << " " << nameS << "=\\\"" << valueS << "\\\"";
   }

   DOMNodeList* contList = el->getChildNodes();
   int contListLength = contList->getLength();
   if (contListLength > 0)
   {
      pyFile << ">\\n\"" << std::endl;
      indent++;
      for (int c = 0; c < contListLength; c++)
      {
         DOMNode* node = contList->item(c);
         if (node->getNodeType() == DOMNode::ELEMENT_NODE)
         {
            DOMElement* contEl = (DOMElement*) node;
            constructDocument(contEl);
         }
      }
      indent--;
      pyFile << "\"";
      for (int n = 0; n < indent; n++)
      {
         pyFile << "  ";
      }
      pyFile << "</" << tagS << ">\\n\"" << std::endl;
   }
   else
   {
      pyFile << " />\\n\"" << std::endl;
   }
}

std::string guessType(const std::string &literal)
{
   const char *str = literal.c_str();
   char *endptr;
   errno=0;
   long long int llvalue = strtoll(str,&endptr,0);
   if (errno == 0 && *endptr == 0) {
      errno=0;
      int lvalue = strtol(str,&endptr,0);
      if (errno == 0 && *endptr == 0 && lvalue == llvalue) {
         return "int";
      }
      else {
         return "long";
      }
   }
   errno=0;
   strtof(str,&endptr);
   if (errno == 0 && *endptr == 0) {
      return "float";
   }
   errno=0;
   strtod(str,&endptr);
   if (errno == 0 && *endptr == 0) {
      return "double";
   }
   if (literal == "true" || literal == "false") {
      return "boolean";
   }
   if ((int)lookupParticle(literal) != 0) {
      return "Particle_t";
   }
   if (XMLUri::isValidURI(false,X(literal))) {
      return "anyURI";
   }
   return "string";
}

Particle_t lookupParticle(const std::string &name)
{
   for (int p=0; p<100; ++p) {
      if (ParticleType((Particle_t)p) == name) {
         return (Particle_t)p;
      }
   }
   return Unknown;
}
