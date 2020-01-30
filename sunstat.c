/*
 * Simple SUNRISET application
 *
 * (c) Paul Schlyter, 1989 - December 1992, released to the public domain
 * (c) Joachim Nilsson, December 2017, released to the public domain
 * (c) David Goerger, January 2020, released to the public domain
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <getopt.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "config.h"
#include "sunriset.h"

static time_t now;
static struct tm *tm;
extern char *__progname;

static int usage(int code)
{
	printf("Usage:\n"
	    "  %s +/-latitude +/-longitude\n"
	    "\n"
	    "Examples:\n"
	    "    %s +40.6611 -73.9439 (use $TZ || /etc/localtime)\n"
	    "    TZ='America/New_York' %s +40.6611 -73.9439\n"
	    "    TZ='UTC' %s +40.6611 -73.9439\n"
	    "\n", __progname, __progname, __progname, __progname);

	return code;
}

static time_t timediff(void)
{
	static time_t diff;

	diff = tm->tm_gmtoff;

	return diff;
}

static void convert(double ut, int *h, int *m)
{
	/*
	 * NOTE: this doesn't reflow hours/minutes
	 *        when coordinates aren't within TZ,
	 *        e.g. TZ='Asia/Tokyo' => sunset
	 *        in New York City occurs "30:43 JST"
	 */

	*h = (int)floor(ut);
	*m = (int)(60 * (ut - floor(ut)));

	*m += (timediff() % 3600)/60;
	*h += timediff() / 3600;
}

static char *lctime_r(double ut, char *buf, size_t len)
{
	int h, m;

	convert(ut, &h, &m);
	snprintf(buf, len, "%02d:%02d", h, m);

	return buf;
}

static char *lctime(double ut)
{
	static char buf[10];

	return lctime_r(ut, buf, sizeof(buf));
}

static char *hours_to_s(double ut)
{
	int h, m = 0, s = 0;
	static char buf[10];

	h = (int)floor(ut);
	m = (int)(60 * (ut - floor(ut)));
	s = (int)(60 * ((60 * (ut - floor(ut))) - m));

	snprintf(buf, sizeof(buf), "%02dh%02dm%02ds", h, m, s);
	return buf;
}

static int all(double lat, double lon, int year, int month, int day)
{
	double daylen, civlen, nautlen, astrlen;
	double rise, set, civ_start, civ_end, naut_start, naut_end;
	double astr_start, astr_end;
	int rs, civ, naut, astr;
	char bufr[10], bufs[10];

	daylen = day_length(year, month, day, lon, lat);
	civlen = day_civil_twilight_length(year, month, day, lon, lat);
	nautlen = day_nautical_twilight_length(year, month, day, lon, lat);
	astrlen = day_astronomical_twilight_length(year, month, day, lon, lat);

	rs = sun_rise_set(year, month, day, lon, lat, &rise, &set);
	civ = civil_twilight(year, month, day, lon, lat, &civ_start, &civ_end);
	naut = nautical_twilight(year, month, day, lon, lat, &naut_start,
	    &naut_end);
	astr = astronomical_twilight(year, month, day, lon, lat, &astr_start,
	    &astr_end);

	printf("                       Sunrise     Sunset\n");
	switch (rs) {
	case 0:
		printf("                       %s %s   %s %s\n",
		    lctime_r(rise, bufr, sizeof(bufr)), tm->tm_zone,
		    lctime_r(set, bufs, sizeof(bufs)), tm->tm_zone);
		break;

	case +1:
		printf("                       ---         (none)\n");
		break;

	case -1:
		printf("                       (none)      ---\n");
		break;
	}

	switch (civ) {
	case 0:
		printf("       Civil twilight  %s %s   %s %s\n",
		    lctime_r(civ_start, bufr, sizeof(bufr)), tm->tm_zone,
		    lctime_r(civ_end, bufs, sizeof(bufs)), tm->tm_zone);
		break;

	case +1:
		printf("       Civil twilight  ---         (none)\n");
		break;

	case -1:
		printf("       Civil twilight  (none)      ---\n");
		break;
	}

	switch (naut) {
	case 0:
		printf("    Nautical twilight  %s %s   %s %s\n",
		    lctime_r(naut_start, bufr, sizeof(bufr)), tm->tm_zone,
		    lctime_r(naut_end, bufs, sizeof(bufs)), tm->tm_zone);
		break;

	case +1:
		printf("    Nautical twilight  ---         (none)\n");
		break;

	case -1:
		printf("    Nautical twilight  (none)      ---\n");
		break;
	}

	switch (astr) {
	case 0:
		printf("Astronomical twilight  %s %s   %s %s\n\n",
		    lctime_r(astr_start, bufr, sizeof(bufr)), tm->tm_zone,
		    lctime_r(astr_end, bufs, sizeof(bufs)), tm->tm_zone);
		break;

	case +1:
		printf("Astronomical twilight  ---         (none)\n\n");
		break;

	case -1:
		printf("Astronomical twilight  (none)      ---\n\n");
		break;
	}

	printf("Hours of daylight, incl. civil twilight: %s.\n",
	    hours_to_s(civlen));
	printf("The Sun is overhead (due south/north) at %s %s.\n",
	    lctime((rise + set) / 2.0), tm->tm_zone);
	return 0;
}

int main(int argc, char *argv[])
{
#ifdef HAVE_PLEDGE
	if (pledge("stdio", NULL) == -1)
		err(1, "pledge");
#endif

	int year = 2000, month = 1, day = 1;
	double lon = 0.0, lat = 0.0;

	now = time(NULL);
	tm = localtime(&now);

	if (optind < argc) {
		lat = atof(argv[optind++]);
	} else {
		return usage(1);
	}
	if (optind < argc) {
		lon = atof(argv[optind]);
	} else {
		return usage(1);
	}

	year = 1900 + tm->tm_year;
	month = 1 + tm->tm_mon;
	day = tm->tm_mday;

	return all(lat, lon, year, month, day);
}

int __sunriset__(int year, int month, int day, double lon, double lat,
    double altit, int upper_limb, double *trise, double *tset)
{
/*
 * Note: year,month,date = calendar date, 1801-2099 only.
 *       Eastern longitude positive, Western longitude negative
 *       Northern latitude positive, Southern latitude negative
 *       The longitude value IS critical in this function!
 *       altit = the altitude which the Sun should cross
 *               Set to -35/60 degrees for rise/set, -6 degrees
 *               for civil, -12 degrees for nautical and -18
 *               degrees for astronomical twilight.
 *         upper_limb: non-zero -> upper limb, zero -> center
 *               Set to non-zero (e.g. 1) when computing rise/set
 *               times, and to zero when computing start/end of
 *               twilight.
 *        *rise = where to store the rise time
 *        *set  = where to store the set  time
 *                Both times are relative to the specified altitude,
 *                and thus this function can be used to compute
 *                various twilight times, as well as rise/set times
 * Return value:  0 = sun rises/sets this day, times stored at
 *                    *trise and *tset.
 *               +1 = sun above the specified "horizon" 24 hours.
 *                    *trise set to time when the sun is at south,
 *                    minus 12 hours while *tset is set to the south
 *                    time plus 12 hours. "Day" length = 24 hours
 *               -1 = sun is below the specified "horizon" 24 hours
 *                    "Day" length = 0 hours, *trise and *tset are
 *                    both set to the time when the sun is at south.
 */
	double  d,  /* Days since 2000 Jan 0.0 (negative before) */
	sr,         /* Solar distance, astronomical units */
	sRA,        /* Sun's Right Ascension */
	sdec,       /* Sun's declination */
	sradius,    /* Sun's apparent radius */
	t,          /* Diurnal arc */
	tsouth,     /* Time when Sun is at south */
	sidtime;    /* Local sidereal time */

	int rc = 0; /* Return cde from function - usually 0 */

	/* Compute d of 12h local mean solar time */
	d = days_since_2000_Jan_0(year,month,day) + 0.5 - lon/360.0;

	/* Compute the local sidereal time of this moment */
	sidtime = revolution(GMST0(d) + 180.0 + lon);

	/* Compute Sun's RA, Decl and distance at this moment */
	sun_RA_dec(d, &sRA, &sdec, &sr);

	/* Compute time when Sun is at south - in hours UTC */
	tsouth = 12.0 - rev180(sidtime - sRA)/15.0;

	/* Compute the Sun's apparent radius in degrees */
	sradius = 0.2666 / sr;

	/* Do correction to upper limb, if necessary */
	if (upper_limb)
		altit -= sradius;

	/* Compute the diurnal arc that the Sun traverses to reach */
	/* the specified altitude altit: */
	{
		double cost;
		cost = (sind(altit) - sind(lat) * sind(sdec)) /
		    (cosd(lat) * cosd(sdec));
		if (cost >= 1.0)
			rc = -1, t = 0.0;       /* Sun always below altit */
		else if (cost <= -1.0)
			rc = +1, t = 12.0;	/* Sun always above altit */
		else
			t = acosd(cost)/15.0;   /* The diurnal arc, hours */
	}

	/* Store rise and set times - in hours UTC */
	*trise = tsouth - t;
	*tset  = tsouth + t;

	return rc;
}

double __daylen__(int year, int month, int day, double lon, double lat,
    double altit, int upper_limb)
{
/*
 * Note: year,month,date = calendar date, 1801-2099 only.
 *       Eastern longitude positive, Western longitude negative
 *       Northern latitude positive, Southern latitude negative
 *       The longitude value is not critical. Set it to the correct
 *       longitude if you're picky, otherwise set to to, say, 0.0
 *       The latitude however IS critical - be sure to get it correct
 *       altit = the altitude which the Sun should cross
 *               Set to -35/60 degrees for rise/set, -6 degrees
 *               for civil, -12 degrees for nautical and -18
 *               degrees for astronomical twilight.
 *         upper_limb: non-zero -> upper limb, zero -> center
 *               Set to non-zero (e.g. 1) when computing day length
 *               and to zero when computing day+twilight length.
 */
	double  d,  /* Days since 2000 Jan 0.0 (negative before) */
	obl_ecl,    /* Obliquity (inclination) of Earth's axis */
	sr,         /* Solar distance, astronomical units */
	slon,       /* True solar longitude */
	sin_sdecl,  /* Sine of Sun's declination */
	cos_sdecl,  /* Cosine of Sun's declination */
	sradius,    /* Sun's apparent radius */
	t;          /* Diurnal arc */

	/* Compute d of 12h local mean solar time */
	d = days_since_2000_Jan_0(year,month,day) + 0.5 - lon/360.0;

	/* Compute obliquity of ecliptic (inclination of Earth's axis) */
	obl_ecl = 23.4393 - 3.563E-7 * d;

	/* Compute Sun's ecliptic longitude and distance */
	sunpos(d, &slon, &sr);

	/* Compute sine and cosine of Sun's declination */
	sin_sdecl = sind(obl_ecl) * sind(slon);
	cos_sdecl = sqrt(1.0 - sin_sdecl * sin_sdecl);

	/* Compute the Sun's apparent radius, degrees */
	sradius = 0.2666 / sr;

	/* Do correction to upper limb, if necessary */
	if (upper_limb)
		altit -= sradius;

	/* Compute the diurnal arc that the Sun traverses to reach */
	/* the specified altitude altit: */
	{
		double cost;
		cost = (sind(altit) - sind(lat) * sin_sdecl) /
		    (cosd(lat) * cos_sdecl);
		if (cost >= 1.0)
			t = 0.0;                      /* Sun always below altit */
		else if (cost <= -1.0)
			t = 24.0;                     /* Sun always above altit */
		else
			t = (2.0/15.0) * acosd(cost); /* The diurnal arc, hours */
	}
	return t;
}

void sunpos(double d, double *lon, double *r)
{
/*
 * Computes the Sun's ecliptic longitude and distance
 * at an instant given in d, number of days since
 * 2000 Jan 0.0.  The Sun's ecliptic latitude is not
 * computed, since it's always very near 0.
 */
	double M,         /* Mean anomaly of the Sun */
	       w,         /* Mean longitude of perihelion */
	       e,         /* Eccentricity of Earth's orbit */
	       E,         /* Eccentric anomaly */
	       x, y,      /* x, y coordinates in orbit */
	       v;         /* True anomaly */

	/* Compute mean elements */
	M = revolution(356.0470 + 0.9856002585 * d);
	w = 282.9404 + 4.70935E-5 * d;
	e = 0.016709 - 1.151E-9 * d;

	/* Compute true longitude and radius vector */
	E = M + e * RADEG * sind(M) * (1.0 + e * cosd(M));
	x = cosd(E) - e;
	y = sqrt(1.0 - e*e) * sind(E);
	*r = sqrt(x*x + y*y);              /* Solar distance */
	v = atan2d(y, x);                  /* True anomaly */
	*lon = v + w;                      /* True solar longitude */
	if (*lon >= 360.0)
		*lon -= 360.0;             /* Make it 0..360 degrees */
}

void sun_RA_dec(double d, double *RA, double *dec, double *r)
{
/*
 * Computes the Sun's equatorial coordinates RA, Decl
 * and also its distance, at an instant given in d,
 * the number of days since 2000 Jan 0.0.
 */
	double lon, obl_ecl, x, y, z;

	/* Compute Sun's ecliptical coordinates */
	sunpos(d, &lon, r);

	/* Compute ecliptic rectangular coordinates (z=0) */
	x = *r * cosd(lon);
	y = *r * sind(lon);

	/* Compute obliquity of ecliptic (inclination of Earth's axis) */
	obl_ecl = 23.4393 - 3.563E-7 * d;

	/* Convert to equatorial rectangular coordinates - x is unchanged */
	z = y * sind(obl_ecl);
	y = y * cosd(obl_ecl);

	/* Convert to spherical coordinates */
	*RA = atan2d(y, x);
	*dec = atan2d(z, sqrt(x*x + y*y));

}

#define INV360	(1.0 / 360.0)

double revolution(double x)
{
/*
 * Reduce angle to within 0..360 degrees
 */
	return(x - 360.0 * floor(x * INV360));
}

double rev180(double x)
{
/*
 * Reduce angle to within +180..+180 degrees
 */
	return(x - 360.0 * floor(x * INV360 + 0.5));
}

double GMST0(double d)
{
/*
 * This function computes GMST0, the Greenwich Mean Sidereal Time
 * at 0h UTC (i.e. the sidereal time at the Greenwich meridian at
 * 0h UTC).  GMST is then the sidereal time at Greenwich at any
 * time of the day.  I've generalized GMST0 as well, and define it
 * as:  GMST0 = GMST - UTC  --  this allows GMST0 to be computed at
 * other times than 0h UTC as well.  While this sounds somewhat
 * contradictory, it is very practical:  instead of computing
 * GMST like:
 *
 *  GMST = (GMST0) + UTC * (366.2422/365.2422)
 *
 * where (GMST0) is the GMST last time UTC was 0 hours, one simply
 * computes:
 *
 *  GMST = GMST0 + UTC
 *
 * where GMST0 is the GMST "at 0h UTC" but at the current moment!
 * Defined in this way, GMST0 will increase with about 4 min a
 * day.  It also happens that GMST0 (in degrees, 1 hr = 15 degr)
 * is equal to the Sun's mean longitude plus/minus 180 degrees!
 * (if we neglect aberration, which amounts to 20 seconds of arc
 * or 1.33 seconds of time)
 *
 * sidtime at 0h UTC = L (Sun's mean longitude) + 180.0 degrees
 * L = M + w, as defined in sunpos().  Since I'm too lazy to
 * add these numbers, I'll let the C compiler do it for me.
 */
	double sidtim0;
	sidtim0 = revolution((180.0 + 356.0470 + 282.9404) +
	    (0.9856002585 + 4.70935E-5) * d);
	return sidtim0;
}
