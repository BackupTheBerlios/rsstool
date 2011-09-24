<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:media="http://search.yahoo.com/mrss/" xmlns:rsscache="data:,rsscache" xmlns:cms="data:,cms">
<xsl:output method="text" omit-xml-declaration="yes"/>
<xsl:template match="/">
-- -----------------------------------------------------------
-- RSStool - read, parse, merge and write RSS and Atom feeds
-- -----------------------------------------------------------

-- DROP TABLE IF EXISTS rsstool_table;
-- CREATE TABLE rsstool_table (
--   rsstool_url_md5 varchar(32) NOT NULL default '',
--   rsstool_url_crc32 int(10) unsigned NOT NULL default '0',
--   rsstool_site text NOT NULL,
--   rsstool_dl_url text NOT NULL,
--   rsstool_dl_url_md5 varchar(32) NOT NULL default '',
--   rsstool_dl_url_crc32 int(10) unsigned NOT NULL default '0',
--   rsstool_title text NOT NULL,
--   rsstool_title_md5 varchar(32) NOT NULL default '',
--   rsstool_title_crc32 int(10) unsigned NOT NULL default '0',
--   rsstool_desc text NOT NULL,
--   rsstool_date bigint(20) unsigned NOT NULL default '0',
--   rsstool_dl_date bigint(20) unsigned NOT NULL default '0',
--   rsstool_keywords text NOT NULL,
--   rsstool_media_duration bigint(20) unsigned NOT NULL default '0',
--   rsstool_image text NOT NULL,
--   rsstool_event_start bigint(20) unsigned NOT NULL default '0',
--   rsstool_event_end bigint(20) unsigned NOT NULL default '0',
--   UNIQUE KEY rsstool_url_crc32 (rsstool_url_crc32),
--   UNIQUE KEY rsstool_url_md5 (rsstool_url_md5),
--   UNIQUE KEY rsstool_title_crc32 (rsstool_title_crc32),
--   UNIQUE KEY rsstool_title_md5 (rsstool_title_md5),
--   FULLTEXT KEY rsstool_title (rsstool_title),
--   FULLTEXT KEY rsstool_desc (rsstool_desc)
-- ) TYPE=MyISAM;

-- DROP TABLE IF EXISTS keyword_table;
-- CREATE TABLE IF NOT EXISTS keyword_table (
--   rsstool_url_md5 varchar(32) NOT NULL,
--   rsstool_url_crc32 int(10) unsigned NOT NULL,
--   rsstool_keyword_crc32 int(10) unsigned NOT NULL,
--   rsstool_keyword_crc24 int(10) unsigned NOT NULL,
--   rsstool_keyword_crc16 smallint(5) unsigned NOT NULL,
--   PRIMARY KEY (rsstool_url_crc32,rsstool_keyword_crc16),
--   KEY rsstool_keyword_24bit (rsstool_keyword_crc24),
--   KEY rsstool_keyword_16bit (rsstool_keyword_crc16)
-- ) ENGINE=MyISAM DEFAULT CHARSET=utf8;

<xsl:for-each select="rss/channel/item">

# <xsl:value-of disable-output-escaping="yes" select="title"/>
<xsl:value-of disable-output-escaping="yes" select="link"/><xsl:text>
</xsl:text><xsl:value-of disable-output-escaping="yes" select="rsscache:download"/><xsl:text>
</xsl:text>




<!--


function
rsstool_write_ansisql ($xml, $rsscache_category, $table_suffix = NULL, $db_conn = NULL)
{
  $sql_update = 0;
  $rsscache_engine = 1;
  $p = '';

  $rsstool_table = 'rsstool_table';
  $keyword_table = 'keyword_table';
  if ($table_suffix)
    if (trim ($table_suffix) != '')
      {   
        $rsstool_table .= '_'.$table_suffix;
        $keyword_table .= '_'.$table_suffix;
      }

  $items = count ($xml->item);
  for ($i = 0; $i < $items; $i++)
    if ($xml->item[$i]->url != '')
    {
      // rsstool_table
      $p .= 'INSERT IGNORE INTO '.$rsstool_table.' ('
           .' rsstool_dl_url,'
//           .' rsstool_dl_url_md5,'
           .' rsstool_dl_url_crc32,'
           .' rsstool_dl_date,'
           .' rsstool_site,'
           .' rsstool_url,'
//           .' rsstool_url_md5,'
           .' rsstool_url_crc32,'
           .' rsstool_date,'
           .' rsstool_title,'
//           .' rsstool_title_md5,'
           .' rsstool_title_crc32,'
           .' rsstool_desc,'
           .' rsstool_keywords,'
           .' rsstool_related_id,'
           .' rsstool_media_duration,'
           .' rsstool_image,'
           .' rsstool_user,'
           .' rsstool_event_start,'
           .' rsstool_event_end';

      // HACK: rsscache category
      if ($rsscache_engine == 1)
        $p .= ', tv2_category, tv2_moved';

      $p .= ' ) VALUES ('
           .' \''.misc_sql_stresc ($xml->item[$i]->dl_url, $db_conn).'\','
//           .' \''.$xml->item[$i]->dl_url_md5.'\','
           .' \''.$xml->item[$i]->dl_url_crc32.'\','
           .' \''.$xml->item[$i]->dl_date.'\','
           .' \''.misc_sql_stresc ($xml->item[$i]->site, $db_conn).'\','
           .' \''.misc_sql_stresc ($xml->item[$i]->url, $db_conn).'\','
//           .' \''.$xml->item[$i]->url_md5.'\','
           .' \''.$xml->item[$i]->url_crc32.'\','
           .' \''.$xml->item[$i]->date.'\','
           .' \''.misc_sql_stresc ($xml->item[$i]->title, $db_conn).'\','
//           .' \''.$xml->item[$i]->title_md5.'\','
           .' \''.$xml->item[$i]->title_crc32.'\','
           .' \''.misc_sql_stresc ($xml->item[$i]->desc, $db_conn).'\','
           .' \''.misc_sql_stresc ($xml->item[$i]->media_keywords, $db_conn).'\','
           .' '.sprintf ("%u", misc_related_string_id ($xml->item[$i]->title)).','
           .' \''.($xml->item[$i]->media_duration * 1).'\','
           .' \''.$xml->item[$i]->image.'\','  
           .' \''.$xml->item[$i]->user.'\','  
           .' \''.($xml->item[$i]->event_start * 1).'\','
           .' \''.($xml->item[$i]->event_end * 1).'\'';

      // HACK: rsscache category
      if ($rsscache_engine == 1)
        $p .= ', \''.$rsscache_category.'\', \''.$rsscache_category.'\'';

      $p .= ' );'."\n";

      // UPDATE rsstool_table
      $p .= '-- just update if row exists'."\n";
      if ($sql_update == 0)
        $p .= '-- ';
      $p .= 'UPDATE '.$rsstool_table.' SET '
           .' rsstool_title = \''.misc_sql_stresc ($xml->item[$i]->title, $db_conn).'\','
//           .' rsstool_title_md5 = \''.$xml->item[$i]->title_md5.'\','
           .' rsstool_title_crc32 = \''.$xml->item[$i]->title_crc32.'\','
           .' rsstool_desc = \''.misc_sql_stresc ($xml->item[$i]->desc, $db_conn).'\''
           .' WHERE rsstool_url_crc32 = '.$xml->item[$i]->url_crc32
           .';'
           ."\n";

      // keyword_table
      $a = explode (' ', $xml->item[$i]->media_keywords);
      for ($j = 0; isset ($a[$j]); $j++)
        if (trim ($a[$j]) != '')
          $p .= 'INSERT IGNORE INTO '.$keyword_table.' ('
//               .' rsstool_url_md5,'
               .' rsstool_url_crc32,'
//               .' rsstool_keyword_crc32,'
//               .' rsstool_keyword_crc24,'
               .' rsstool_keyword_crc16'
               .' ) VALUES ('
//               .' \''.$xml->item[$i]->url_md5.'\','
               .' '.$xml->item[$i]->url_crc32.','
//               .' '.sprintf ("%u", crc32 ($a[$j])).','
//               .' '.sprintf ("%u", misc_crc24 ($a[$j])).','
               .' '.misc_crc16 ($a[$j])
               .' );'
               ."\n";
    }

  return $p;
}

-->
</xsl:for-each>
</xsl:template>
</xsl:stylesheet>
