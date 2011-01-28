#ifndef GMAC_UTIL_SINGLETON_H_
#define GMAC_UTIL_SINGLETON_H_

#include "config/common.h"

/*
 *
 */
namespace __impl { namespace util {

template <typename T>
class GMAC_LOCAL Singleton {
private:
	static T *Singleton_;
protected:
	Singleton();
public:
	virtual ~Singleton();

	template <typename U>
	static void create();
	static void destroy();
	static T& getInstance();
};

}}

#include "Singleton-impl.h"

#endif /* SINGLETON_H_ */
