#ifndef _FUNCS_H_INCLUDED_
#define _FUNCS_H_INCLUDED_


tagged_value log_tv(arglist_member* arg);
tagged_value sin_tv(arglist_member* arg);
tagged_value cos_tv(arglist_member* arg);
tagged_value tan_tv(arglist_member* arg);
tagged_value asin_tv(arglist_member* arg);
tagged_value acos_tv(arglist_member* arg);
tagged_value atan_tv(arglist_member* arg);
tagged_value abs_tv(arglist_member* arg);
tagged_value floor_tv(arglist_member* arg);
tagged_value ceil_tv(arglist_member* arg);
tagged_value sqrt_tv(arglist_member* arg);
tagged_value print_tv(arglist_member* arg);
tagged_value size_tv(arglist_member* arg);
tagged_value char_tv(arglist_member* arg);
tagged_value rand_tv(arglist_member* arg);
tagged_value prng_tv(arglist_member *arg);
tagged_value getpersistent(arglist_member *arg);
tagged_value setpersistent(arglist_member *arg);
tagged_value type_tv(arglist_member *arg);
tagged_value mandelbrot_tv(arglist_member *arg);

#endif // _FUNCS_H_INCLUDED_
