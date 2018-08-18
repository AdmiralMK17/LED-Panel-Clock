bool SunLight::CalculateDaylightStatus()
{
  // Calculate the current time of day.
  time_t currentTime = time(NULL);
  m_LocalTime = localtime(&currentTime);

  // Initialize the sunrise and set times.
  *m_Sunrise = *m_LocalTime;
  *m_Sunset = *m_LocalTime;

  // Flags to check whether sunrise or set available on the day or not.
  m_IsSunrise = false;
  m_IsSunset = false;

  m_RiseAzimuth = 0.0;
  m_SetAzimuth = 0.0;

  for (unsigned int i = 0; i < 3; i++)
  {
    m_RightAscention[i] = 0.0;
    m_Decension[i] = 0.0;
    m_VHz[i] = 0.0;
  }

  for (unsigned int i = 0; i < 2; i++)
  {
    m_SunPositionInSky[i] = 0.0;
    m_RiseTime[i] = 0;
    m_SetTime[i] = 0;
  }

  // Calculate the sunrise and set times.
  CalculateSunRiseSetTimes();

  return (mktime(m_LocalTime) >= mktime(m_Sunrise) && mktime(m_LocalTime) < mktime(m_Sunset))
             ? true
             : false;
}
//---------------------------------------------------------------------

bool SunLight::CalculateSunRiseSetTimes()
{
  double zone = timezone / 3600 - m_LocalTime->tm_isdst;

  // Julian day relative to Jan 1.5, 2000.
  double jd = GetJulianDay() - 2451545;

  if ((Sign(zone) == Sign(m_Config->Longitude())) && (zone != 0))
  {
    return false;
  }

  double tz = zone / 24;

  // Centuries since 1900.0
  double ct = jd / 36525 + 1;

  // Local sidereal time.
  double t0 = LocalSiderealTimeForTimeZone(jd, tz, m_Config->Longitude() / 360);

  // Get sun position at start of day.
  jd += tz;

  // Calculate the position of the sun.
  CalculateSunPosition(jd, ct);

  double ra0 = m_SunPositionInSky[0];
  double dec0 = m_SunPositionInSky[1];

  // Get sun position at end of day.
  jd += 1;

  // Calculate the position of the sun.
  CalculateSunPosition(jd, ct);

  double ra1 = m_SunPositionInSky[0];
  double dec1 = m_SunPositionInSky[1];

  // make continuous
  if (ra1 < ra0)
    ra1 += 2 * M_PI;

  m_RightAscention[0] = ra0;
  m_Decension[0] = dec0;

  // check each hour of this day
  for (int k = 0; k < 24; k++)
  {
    m_RightAscention[2] = ra0 + (k + 1) * (ra1 - ra0) / 24;
    m_Decension[2] = dec0 + (k + 1) * (dec1 - dec0) / 24;
    m_VHz[2] = TestHour(k, t0, m_Config->Latitude());

    // advance to next hour
    m_RightAscention[0] = m_RightAscention[2];
    m_Decension[0] = m_Decension[2];
    m_VHz[0] = m_VHz[2];
  }

  // Update the tm structure with time values.
  m_Sunrise->tm_hour = m_RiseTime[0];
  m_Sunrise->tm_min = m_RiseTime[1];

  m_Sunset->tm_hour = m_SetTime[0];
  m_Sunset->tm_min = m_SetTime[1];

  // neither sunrise nor sunset
  if ((!m_IsSunrise) && (!m_IsSunset))
  {
    // Sun down all day.
    if (m_VHz[2] < 0)
      m_IsSunset = true;

    // Sun up all day.
    else
      m_IsSunrise = true;
  }
  return true;
}
//---------------------------------------------------------------------

int SunLight::Sign(double value)
{
  if (value > 0.0)
    return 1;
  else if (value < 0.0)
    return -1;
  else
    return 0;
}
//---------------------------------------------------------------------

// Local Sidereal Time for zone.
double
SunLight::LocalSiderealTimeForTimeZone(double jd, double z, double lon)
{
  double s = 24110.5 + 8640184.812999999 * jd / 36525 + 86636.6 * z + 86400 * lon;
  s = s / 86400;
  s = s - floor(s);
  return s * 360 * cDegToRad;
}
//---------------------------------------------------------------------

// Determine Julian day from calendar date
// (Jean Meeus, "Astronomical Algorithms", Willmann-Bell, 1991).
double
SunLight::GetJulianDay()
{
  int month = m_LocalTime->tm_mon + 1;
  int day = m_LocalTime->tm_mday;
  int year = 1900 + m_LocalTime->tm_year;

  bool gregorian = (year < 1583) ? false : true;

  if ((month == 1) || (month == 2))
  {
    year = year - 1;
    month = month + 12;
  }

  double a = floor((double)year / 100);
  double b = 0;

  if (gregorian)
    b = 2 - a + floor(a / 4);
  else
    b = 0.0;

  double jd = floor(365.25 * (year + 4716)) + floor(30.6001 * (month + 1)) + day + b - 1524.5;

  return jd;
}
//---------------------------------------------------------------------

// Sun position using fundamental arguments
// (Van Flandern & Pulkkinen, 1979).
void SunLight::CalculateSunPosition(double jd, double ct)
{
  double g, lo, s, u, v, w;

  lo = 0.779072 + 0.00273790931 * jd;
  lo = lo - floor(lo);
  lo = lo * 2 * M_PI;

  g = 0.993126 + 0.0027377785 * jd;
  g = g - floor(g);
  g = g * 2 * M_PI;

  v = 0.39785 * sin(lo);
  v = v - 0.01 * sin(lo - g);
  v = v + 0.00333 * sin(lo + g);
  v = v - 0.00021 * ct * sin(lo);

  u = 1 - 0.03349 * cos(g);
  u = u - 0.00014 * cos(2 * lo);
  u = u + 0.00008 * cos(lo);

  w = -0.0001 - 0.04129 * sin(2 * lo);
  w = w + 0.03211 * sin(g);
  w = w + 0.00104 * sin(2 * lo - g);
  w = w - 0.00035 * sin(2 * lo + g);
  w = w - 0.00008 * ct * sin(g);

  // compute sun right ascension
  s = w / sqrt(u - v * v);
  m_SunPositionInSky[0] = lo + atan(s / sqrt(1 - s * s));

  // ...and declination
  s = v / sqrt(u);
  m_SunPositionInSky[1] = atan(s / sqrt(1 - s * s));
}
//---------------------------------------------------------------------

// Test an hour for an event.
double
SunLight::TestHour(int k, double t0, double prmLatitude)
{
  double ha[3];
  double a, b, c, d, e, s, z;
  double time;
  double az, dz, hz, nz;
  int hr, min;

  ha[0] = t0 - m_RightAscention[0] + k * cK1;
  ha[2] = t0 - m_RightAscention[2] + k * cK1 + cK1;

  ha[1] = (ha[2] + ha[0]) / 2;                            // hour angle at half hour
  m_Decension[1] = (m_Decension[2] + m_Decension[0]) / 2; // declination at half hour

  s = sin(prmLatitude * cDegToRad);
  c = cos(prmLatitude * cDegToRad);
  z = cos(90.833 * cDegToRad); // refraction + sun semi-diameter at horizon

  if (k <= 0)
    m_VHz[0] = s * sin(m_Decension[0]) + c * cos(m_Decension[0]) * cos(ha[0]) - z;

  m_VHz[2] = s * sin(m_Decension[2]) + c * cos(m_Decension[2]) * cos(ha[2]) - z;

  if (Sign(m_VHz[0]) == Sign(m_VHz[2]))
    return m_VHz[2]; // no event this hour

  m_VHz[1] = s * sin(m_Decension[1]) + c * cos(m_Decension[1]) * cos(ha[1]) - z;

  a = 2 * m_VHz[0] - 4 * m_VHz[1] + 2 * m_VHz[2];
  b = -3 * m_VHz[0] + 4 * m_VHz[1] - m_VHz[2];
  d = b * b - 4 * a * m_VHz[0];

  if (d < 0)
    return m_VHz[2]; // no event this hour

  d = sqrt(d);
  e = (-b + d) / (2 * a);

  if ((e > 1) || (e < 0))
    e = (-b - d) / (2 * a);

  time = (double)k + e + (double)1 / (double)120; // time of an event

  hr = (int)floor(time);
  min = (int)floor((time - hr) * 60);

  hz = ha[0] + e * (ha[2] - ha[0]); // azimuth of the sun at the event
  nz = -cos(m_Decension[1]) * sin(hz);
  dz = c * sin(m_Decension[1]) - s * cos(m_Decension[1]) * cos(hz);
  az = atan2(nz, dz) / cDegToRad;
  if (az < 0)
    az = az + 360;

  if ((m_VHz[0] < 0) && (m_VHz[2] > 0))
  {
    m_RiseTime[0] = hr;
    m_RiseTime[1] = min;
    m_RiseAzimuth = az;
    m_IsSunrise = true;
  }

  if ((m_VHz[0] > 0) && (m_VHz[2] < 0))
  {
    m_SetTime[0] = hr;
    m_SetTime[1] = min;
    m_SetAzimuth = az;
    m_IsSunset = true;
  }

  return m_VHz[2];
}