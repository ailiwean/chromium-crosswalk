<?xml version="1.0" encoding="iso8859-5"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
    <xsl:output method="html" encoding="KOI8-R"/>
    <xsl:template match="TEST">
        <html>
            <body>
              <script>
                if (window.layoutTestController)
                  layoutTestController.dumpAsText();
                
              	//alert(document.characterSet); // works in Firefox
              	document.write("Encoding: " + document.characterSet);
              </script>
            </body>
        </html>
  </xsl:template>

</xsl:stylesheet>
