//enum mclass { mc_f, mc_i, mc_t };

#define mclass FIT_IDX

/*
 *	More comments should eventually come here.  These markers are attached
 *	to units, in an unlimited number.  At present, they carry prosodic information
 *	(when fixed, only intensity and pitch, not duration) relating either to
 *	the unit, or to a specified point within it.
 *
 *	This was added between 2.4 and 2.5.
 */

class marker
{
	friend unit;

	mclass type;
	bool extent;
	int par;
	float pos;
	marker *next;

   public:
	marker();
	~marker();
	marker(mclass mc, bool e, int p, marker *m, float po) { type = mc; extent = e; par = p; next = m; pos = po; };
	marker *derived();
	void merge(marker *ma, marker **fixup);

	bool operator < (marker ma);

	void *operator new(size_t size);
	void operator delete(void *ptr);
};

marker::marker()
{
	next = NULL;
}

marker::~marker()
{
	if (next) delete next;
}

marker *marker::derived()
{
	if (!this) return NULL;
	marker *nm = new marker;
	nm->next = next->derived();
	nm->type = type;
	nm->extent = extent;
	nm->par = par;
	nm->pos = pos;
	return nm;
}

void
marker::merge(marker *ma, marker **fixup)
{
	if (!this) {
		*fixup = ma;
		return;
	}
	if (*this < *ma) {
		if (!next) next = ma;
		else next->merge(ma, &next);
	} else {
		*fixup = ma;
		if (!ma->next) ma->next = this;
		else ma->next->merge(this, &ma->next);
	}
}

bool
marker::operator < (marker ma)
{
	if (extent == ma.extent) return pos < ma.pos;
	else return !extent;
}

