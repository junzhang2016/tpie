// Copyright (c) 1994 Darren Vengroff
//
// File: ami_gen_perm.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 11/1/94
//
// $Id: ami_gen_perm.h,v 1.5 1999-01-22 17:14:55 rajiv Exp $
//
// General permutation.
//
#ifndef _AMI_GEN_PERM_H
#define _AMI_GEN_PERM_H

#include <ami_gen_perm_object.h>

// (tavi) moved dest_obj declaration down due to error in gcc 2.8.1
template<class T> class dest_obj;

// A comparison operator that simply compares destinations (for sorting).
template<class T>
int operator<(const dest_obj<T> &s, const dest_obj<T> &t)
{
    return s.dest < t.dest;
}

template<class T>
int operator>(const dest_obj<T> &s, const dest_obj<T> &t)
{
    return s.dest > t.dest;
}


template<class T>
class gen_perm_add_dest : AMI_scan_object {
private:
    AMI_gen_perm_object *pgp;
    off_t input_offset;
public:
    gen_perm_add_dest(AMI_gen_perm_object *gpo) : pgp(gpo) {};
    virtual ~gen_perm_add_dest(void) {};
    AMI_err initialize(void) { input_offset = 0; return AMI_ERROR_NO_ERROR; };
    AMI_err operate(const T &in, AMI_SCAN_FLAG *sfin, dest_obj<T> *out,
                    AMI_SCAN_FLAG *sfout)
    {
        if (!(*sfout = *sfin)) {
            return AMI_SCAN_DONE;
        }
        *out = dest_obj<T>(in, pgp->destination(input_offset++));
        return AMI_SCAN_CONTINUE;
    }
};
    
template<class T>
class gen_perm_strip_dest : AMI_scan_object {
public:
    AMI_err initialize(void) { return AMI_ERROR_NO_ERROR; };
    AMI_err operate(const dest_obj<T> &in, AMI_SCAN_FLAG *sfin, T *out,
                    AMI_SCAN_FLAG *sfout)
    {
        if (!(*sfout = *sfin)) {
            return AMI_SCAN_DONE;
        }
        *out = in.t;
        return AMI_SCAN_CONTINUE;
    }
};


template<class T>
class dest_obj {
private:
    T t;
    off_t dest;
public:
    dest_obj(void) {};
    dest_obj(T t_in, off_t d) : t(t_in), dest(d) {};
    ~dest_obj(void) {};
  friend int operator< <T>(const dest_obj<T> &s, const dest_obj<T> &t);
    friend int operator> <T>(const dest_obj<T> &s, const dest_obj<T> &t);
    friend gen_perm_strip_dest<T>::operate(const dest_obj<T> &in,
                                           AMI_SCAN_FLAG *sfin, T *out,
                                           AMI_SCAN_FLAG *sfout);
};

template<class T>
AMI_err AMI_general_permute(AMI_STREAM<T> *instream, AMI_STREAM<T> *outstream,
                            AMI_gen_perm_object *gpo) {

    AMI_err ae;
    gen_perm_add_dest<T> gpad(gpo);
    gen_perm_strip_dest<T> gpsd;
    AMI_STREAM< dest_obj<T> > sdo_in;
    AMI_STREAM< dest_obj<T> > sdo_out;

    // Initialize
    ae = gpo->initialize(instream->stream_len());
    if (ae != AMI_ERROR_NO_ERROR) {
        return ae;
    }
    
    // Scan the stream, producing an output stream that labels each
    // item with its destination.
    ae = AMI_scan((AMI_STREAM<T> *)instream, &gpad,
                  (AMI_STREAM< dest_obj<T> > *)&sdo_in);
    if (ae != AMI_ERROR_NO_ERROR) {
        return ae;
    }

    // Sort by destination.
    ae = AMI_sort(&sdo_in, &sdo_out);
    if (ae != AMI_ERROR_NO_ERROR) {
        return ae;
    }

    // Scan to strip off the destinations.
    ae = AMI_scan((AMI_STREAM< dest_obj<T> > *)&sdo_out, &gpsd,
                  (AMI_STREAM<T> *)outstream);
    if (ae != AMI_ERROR_NO_ERROR) {
        return ae;
    }

    return AMI_ERROR_NO_ERROR;        
}


#define TEMPLATE_INSTANTIATE_GEN_PERM(T)				\
template class dest_obj<T>;						\
TEMPLATE_INSTANTIATE_STREAMS(dest_obj<T>)				\
template int operator<(const dest_obj<T> &, const dest_obj<T> &);	\
template int operator>(const dest_obj<T> &, const dest_obj<T> &);	\
TEMPLATE_INSTANTIATE_SORT_OP(dest_obj<T>)				\
template class gen_perm_strip_dest<T>;					\
template class gen_perm_add_dest<T>;					\
template AMI_err AMI_scan(AMI_STREAM<T> *,				\
                          gen_perm_add_dest<T> *,			\
                          AMI_STREAM< dest_obj<T> > *);			\
template AMI_err AMI_scan(AMI_STREAM< dest_obj<T> > *,			\
                          gen_perm_strip_dest<T> *,			\
                          AMI_STREAM<T> *);				\
template AMI_err AMI_general_permute(AMI_STREAM<T> *, AMI_STREAM<T> *,	\
                                     AMI_gen_perm_object *gpo);


#endif // _AMI_GEN_PERM_H 
