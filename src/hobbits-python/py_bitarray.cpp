#include "py_bitarray.h"
#include "bitarray.h"
#include <QSharedPointer>

#include <structmember.h>

#define UNUSED(expr) do { (void)(expr); } while (0);
#define BITS(X) static_cast<BitArray*>(PyCapsule_GetPointer(X, nullptr))

typedef struct {
    PyObject_HEAD
    PyObject* bitsCapsule;
} BitArrayPyObj;

static void BitArrayPy_dealloc(BitArrayPyObj *self)
{
    Py_XDECREF(self->bitsCapsule);
    Py_TYPE(self)->tp_free((PyObject*) self);
}

static PyObject* BitArrayPy_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    UNUSED(args)
    UNUSED(kwds)
    BitArrayPyObj *self;
    self = reinterpret_cast<BitArrayPyObj*>(type->tp_alloc(type, 0));
    if (self != nullptr) {
        self->bitsCapsule = nullptr;
    }
    return (PyObject*) self;
}

static int BitArrayPy_init(BitArrayPyObj *self, PyObject *args, PyObject *kwds)
{
    UNUSED(kwds)
    PyObject *bitArrayCapsule;
    if (!PyArg_ParseTuple(args, "O", &bitArrayCapsule)) {
        PyErr_SetString(PyExc_TypeError, "invalid arguments - requires a bit array capsule");
        return -1;
    }

    Py_INCREF(bitArrayCapsule);
    self->bitsCapsule = bitArrayCapsule;
    return 0;
}

static PyObject* BitArrayPy_size(BitArrayPyObj *self, PyObject *Py_UNUSED(ignored))
{
    BitArray* bits = BITS(self->bitsCapsule);
    return PyLong_FromLongLong(bits->sizeInBits());
}

static PyObject* BitArrayPy_at(BitArrayPyObj *self, PyObject *args )
{
    long long idx;
    if (!PyArg_ParseTuple(args, "L", &idx)) {
        PyErr_SetString(PyExc_TypeError, "invalid arguments - requires a bit index");
        return nullptr;
    }

    BitArray* bits = BITS(self->bitsCapsule);
    if (bits->sizeInBits() <= idx || idx < 0) {
        PyErr_SetString(PyExc_IndexError, "provided bit index is not valid");
        return nullptr;
    }

    return PyBool_FromLong(bits->at(idx));
}

static PyObject* BitArrayPy_resize(BitArrayPyObj *self, PyObject *args )
{
    long long size;
    if (!PyArg_ParseTuple(args, "L", &size)) {
        PyErr_SetString(PyExc_TypeError, "invalid arguments - requires a size in bits");
        return nullptr;
    }

    if (size < 0) {
        PyErr_SetString(PyExc_IndexError, "negative bit array size is not valid");
        return nullptr;
    }

    BitArray* bits = BITS(self->bitsCapsule);
    bits->resize(size);
    Py_RETURN_NONE;
}

static PyObject* BitArrayPy_set(BitArrayPyObj *self, PyObject *args )
{
    long long idx;
    int val;
    if (!PyArg_ParseTuple(args, "Lp", &idx, &val)) {
        PyErr_SetString(PyExc_TypeError, "invalid arguments - requires a bit index and value");
        return nullptr;
    }

    BitArray* bits = BITS(self->bitsCapsule);
    if (bits->sizeInBits() <= idx || idx < 0) {
        PyErr_SetString(PyExc_IndexError, "provided bit index is not valid");
        return nullptr;
    }

    bits->set(idx, val);
    Py_RETURN_NONE;
}

static PyObject* BitArrayPy_read_bytes(BitArrayPyObj *self, PyObject *args )
{
    long long byteOffset;
    long long length;
    if (!PyArg_ParseTuple(args, "LL", &byteOffset, &length)) {
        PyErr_SetString(PyExc_TypeError, "invalid arguments - requires a byte offset and a length");
        return nullptr;
    }

    BitArray* bits = BITS(self->bitsCapsule);
    if (bits->sizeInBytes() <= byteOffset || byteOffset < 0) {
        PyErr_SetString(PyExc_IndexError, "provided byte offset is not valid");
        return nullptr;
    }

    QByteArray bytes = bits->readBytes(byteOffset, length);

    return PyByteArray_FromStringAndSize(bytes.data(), bytes.length());
}

static PyObject* BitArrayPy_set_bytes(BitArrayPyObj *self, PyObject *args )
{
    long long byteOffset;
    Py_buffer bytes;
    if (!PyArg_ParseTuple(args, "Ly*", &byteOffset, &bytes)) {
        PyErr_SetString(PyExc_TypeError, "invalid arguments - requires a byte offset (int) and a bytes-like object");
        return nullptr;
    }
    if (byteOffset < 0) {
        PyErr_SetString(PyExc_IndexError, "invalid argument - cannot have a negative byte offset");
        return nullptr;
    }

    BitArray* bits = BITS(self->bitsCapsule);
    bits->setBytes(byteOffset, reinterpret_cast<const char*>(bytes.buf), 0, bytes.len);

    Py_RETURN_NONE;
}

static PyObject* BitArrayPy_write_to(BitArrayPyObj *self, PyObject *args )
{
    const char *filename;
    if (!PyArg_ParseTuple(args, "s", &filename)) {
        PyErr_SetString(PyExc_TypeError, "invalid arguments - requires a file name");
        return nullptr;
    }

    QFile file(filename);
    if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
        PyErr_SetString(PyExc_TypeError, "file could not be opened for writing");
        return nullptr;
    }

    BitArray* bits = BITS(self->bitsCapsule);
    bits->writeTo(&file);

    file.close();

    Py_RETURN_NONE;
}

static PyObject* BitArrayPy_read_from(BitArrayPyObj *self, PyObject *args )
{
    long long byteOffset;
    const char *filename;
    if (!PyArg_ParseTuple(args, "Ls", &byteOffset, &filename)) {
        PyErr_SetString(PyExc_TypeError, "invalid arguments - requires a byte offset and a file name");
        return nullptr;
    }

    if (byteOffset < 0) {
        PyErr_SetString(PyExc_IndexError, "invalid argument - cannot have a negative byte offset");
        return nullptr;
    }

    QFile file(filename);
    if (!file.open(QFile::ReadOnly)) {
        PyErr_SetString(PyExc_TypeError, "file could not be opened for reading");
        return nullptr;
    }
    QScopedPointer<BitArray> otherBits(new BitArray(&file));
    file.close();


    BitArray* bits = BITS(self->bitsCapsule);
    otherBits->copyBits(0, bits, byteOffset * 8, otherBits->sizeInBits());

    Py_RETURN_NONE;
}

static PyMethodDef BitArrayPy_methods[] = {
    { "size", PyCFunction(BitArrayPy_size), METH_NOARGS, nullptr },
    { "at", PyCFunction(BitArrayPy_at), METH_VARARGS, nullptr },
    { "resize", PyCFunction(BitArrayPy_resize), METH_VARARGS, nullptr },
    { "set", PyCFunction(BitArrayPy_set), METH_VARARGS, nullptr },
    { "read_bytes", PyCFunction(BitArrayPy_read_bytes), METH_VARARGS, nullptr },
    { "set_bytes", PyCFunction(BitArrayPy_set_bytes), METH_VARARGS, nullptr },
    { "write_to", PyCFunction(BitArrayPy_write_to), METH_VARARGS, nullptr },
    { "read_from", PyCFunction(BitArrayPy_read_from), METH_VARARGS, nullptr },
    {}  /* Sentinel */
};

static PyMethodDef ImmutableBitArrayPy_methods[] = {
    { "size", PyCFunction(BitArrayPy_size), METH_NOARGS, nullptr },
    { "at", PyCFunction(BitArrayPy_at), METH_VARARGS, nullptr },
    { "read_bytes", PyCFunction(BitArrayPy_read_bytes), METH_VARARGS, nullptr },
    { "write_to", PyCFunction(BitArrayPy_write_to), METH_VARARGS, nullptr },
    {}  /* Sentinel */
};


static PyMemberDef ImmutableBitArrayPy_members[] = {
    {}  /* Sentinel */
};

static PyMemberDef BitArrayPy_members[] = {
    {}  /* Sentinel */
};

PyTypeObject PyBitArray = {
    PyVarObject_HEAD_INIT(nullptr, 0)

    "hobbits.BitArray", // const char *tp_name; /* For printing, in format "<module>.<name>" */
    sizeof(BitArrayPyObj), // Py_ssize_t tp_basicsize, ; /* For allocation */
    0, //tp_itemsize

    /* Methods to implement standard operations */

    destructor(BitArrayPy_dealloc), // destructor tp_dealloc;
    0, // Py_ssize_t tp_vectorcall_offset;
    nullptr, // getattrfunc tp_getattr;
    nullptr, // setattrfunc tp_setattr;
    nullptr, // PyAsyncMethods *tp_as_async; /* formerly known as tp_compare (Python 2) or tp_reserved (Python 3) */
    nullptr, // reprfunc tp_repr;

    /* Method suites for standard classes */

    nullptr, // PyNumberMethods *tp_as_number;
    nullptr, // PySequenceMethods *tp_as_sequence;
    nullptr, // PyMappingMethods *tp_as_mapping;

    /* More standard operations (here for binary compatibility) */

    nullptr, // hashfunc tp_hash;
    nullptr, // ternaryfunc tp_call;
    nullptr, // reprfunc tp_str;
    nullptr, // getattrofunc tp_getattro;
    nullptr, // setattrofunc tp_setattro;

    /* Functions to access object as input/output buffer */
    nullptr, // PyBufferProcs *tp_as_buffer;

    /* Flags to define presence of optional/expanded features */
    Py_TPFLAGS_DEFAULT, // unsigned long tp_flags;

    "Hobbits Bit Array", // const char *tp_doc; /* Documentation string */

    /* Assigned meaning in release 2.0 */
    /* call function for all accessible objects */
    nullptr, // traverseproc tp_traverse;

    /* delete references to contained objects */
    nullptr, // inquiry tp_clear;

    /* Assigned meaning in release 2.1 */
    /* rich comparisons */
    nullptr, // richcmpfunc tp_richcompare;

    /* weak reference enabler */
    0, // Py_ssize_t tp_weaklistoffset;

    /* Iterators */
    nullptr, // getiterfunc tp_iter;
    nullptr, // iternextfunc tp_iternext;

    /* Attribute descriptor and subclassing stuff */
    BitArrayPy_methods, // struct PyMethodDef *tp_methods;
    BitArrayPy_members, // struct PyMemberDef *tp_members;
    nullptr, // struct PyGetSetDef *tp_getset;
    nullptr, // struct _typeobject *tp_base;
    nullptr, // PyObject *tp_dict;
    nullptr, // descrgetfunc tp_descr_get;
    nullptr, // descrsetfunc tp_descr_set;
    0, // Py_ssize_t tp_dictoffset;
    initproc(BitArrayPy_init), // initproc tp_init;
    nullptr, // allocfunc tp_alloc;
    BitArrayPy_new, // newfunc tp_new;
    nullptr, // freefunc tp_free; /* Low-level free-memory routine */
    nullptr, // inquiry tp_is_gc; /* For PyObject_IS_GC */
    nullptr, // PyObject *tp_bases;
    nullptr, // PyObject *tp_mro; /* method resolution order */
    nullptr, // PyObject *tp_cache;
    nullptr, // PyObject *tp_subclasses;
    nullptr, // PyObject *tp_weaklist;
    nullptr, // destructor tp_del;

    /* Type attribute cache version tag. Added in version 2.6 */
    0, // unsigned int tp_version_tag;

    nullptr, // destructor tp_finalize;
    nullptr, // vectorcallfunc tp_vectorcall;
    // Py_DEPRECATED(3.8) int (*tp_print)(PyObject *, FILE *, int);
};

PyTypeObject PyImmutableBitArray = {
    PyVarObject_HEAD_INIT(nullptr, 0)

    "hobbits.ImmutableBitArray", // const char *tp_name; /* For printing, in format "<module>.<name>" */
    sizeof(BitArrayPyObj), // Py_ssize_t tp_basicsize, ; /* For allocation */
    0, //tp_itemsize

    /* Methods to implement standard operations */

    destructor(BitArrayPy_dealloc), // destructor tp_dealloc;
    0, // Py_ssize_t tp_vectorcall_offset;
    nullptr, // getattrfunc tp_getattr;
    nullptr, // setattrfunc tp_setattr;
    nullptr, // PyAsyncMethods *tp_as_async; /* formerly known as tp_compare (Python 2) or tp_reserved (Python 3) */
    nullptr, // reprfunc tp_repr;

    /* Method suites for standard classes */

    nullptr, // PyNumberMethods *tp_as_number;
    nullptr, // PySequenceMethods *tp_as_sequence;
    nullptr, // PyMappingMethods *tp_as_mapping;

    /* More standard operations (here for binary compatibility) */

    nullptr, // hashfunc tp_hash;
    nullptr, // ternaryfunc tp_call;
    nullptr, // reprfunc tp_str;
    nullptr, // getattrofunc tp_getattro;
    nullptr, // setattrofunc tp_setattro;

    /* Functions to access object as input/output buffer */
    nullptr, // PyBufferProcs *tp_as_buffer;

    /* Flags to define presence of optional/expanded features */
    Py_TPFLAGS_DEFAULT, // unsigned long tp_flags;

    "Immutable Hobbits Bit Array", // const char *tp_doc; /* Documentation string */

    /* Assigned meaning in release 2.0 */
    /* call function for all accessible objects */
    nullptr, // traverseproc tp_traverse;

    /* delete references to contained objects */
    nullptr, // inquiry tp_clear;

    /* Assigned meaning in release 2.1 */
    /* rich comparisons */
    nullptr, // richcmpfunc tp_richcompare;

    /* weak reference enabler */
    0, // Py_ssize_t tp_weaklistoffset;

    /* Iterators */
    nullptr, // getiterfunc tp_iter;
    nullptr, // iternextfunc tp_iternext;

    /* Attribute descriptor and subclassing stuff */
    ImmutableBitArrayPy_methods, // struct PyMethodDef *tp_methods;
    ImmutableBitArrayPy_members, // struct PyMemberDef *tp_members;
    nullptr, // struct PyGetSetDef *tp_getset;
    nullptr, // struct _typeobject *tp_base;
    nullptr, // PyObject *tp_dict;
    nullptr, // descrgetfunc tp_descr_get;
    nullptr, // descrsetfunc tp_descr_set;
    0, // Py_ssize_t tp_dictoffset;
    initproc(BitArrayPy_init), // initproc tp_init;
    nullptr, // allocfunc tp_alloc;
    BitArrayPy_new, // newfunc tp_new;
    nullptr, // freefunc tp_free; /* Low-level free-memory routine */
    nullptr, // inquiry tp_is_gc; /* For PyObject_IS_GC */
    nullptr, // PyObject *tp_bases;
    nullptr, // PyObject *tp_mro; /* method resolution order */
    nullptr, // PyObject *tp_cache;
    nullptr, // PyObject *tp_subclasses;
    nullptr, // PyObject *tp_weaklist;
    nullptr, // destructor tp_del;

    /* Type attribute cache version tag. Added in version 2.6 */
    0, // unsigned int tp_version_tag;

    nullptr, // destructor tp_finalize;
    nullptr, // vectorcallfunc tp_vectorcall;
    // Py_DEPRECATED(3.8) int (*tp_print)(PyObject *, FILE *, int);
};
