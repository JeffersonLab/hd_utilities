#!/apps/python/PRO/bin/python

# pass the table generated from the database info and print google chart output
def detector_o(table, det_label, channel_label, fig_label, num_channels, channel_title, form_num):

    # plot specific dataTable
    table_columns=""
    table_columns+=("   function init() { \n")
    table_columns+=("      var dataTable = new google.visualization.DataTable(); \n")
    table_columns+=("      dataTable.addColumn('string', 'Run'); \n")
    for i in range(1,num_channels+1):
        if not channel_title:
            table_columns+=("      dataTable.addColumn('number', '%s%d'); \n" %(channel_label,i))
        else:
            table_columns+=("      dataTable.addColumn('number', '%s'); \n" % (channel_title[i-1]))
	table_columns+=("      dataTable.addColumn({type: 'string', role: 'annotationText'}); \n")
    print table_columns

    table_rows="""      dataTable.addRows([
%s
        ]);"""
    print table_rows % (table)

    options=""" 
      var options = { 
         'vAxis': {'title': '%s'}, 'hAxis': {'title': 'Run Number'} };
  
      var chart = new google.visualization.LineChart(document.getElementById('%s'));
 
      var button = document.getElementById('%s_button');
      function drawChart() {
         // Disabling the button while the chart is drawing.
         button.disabled = true;
         google.visualization.events.addListener(chart, 'ready',
             function() {
               button.disabled = false;
             });

         chart.draw(dataTable,options);
      }
      drawChart();

      button.onclick = function() {
         var columnArray = [0];
         var i;
         var channel = document.forms[%d].channel;
         for (i = 0; i < channel.length; i++) {
             if (channel[i].checked) {
	         var dataColumnNum = 2*parseInt(channel[i].value-1)+1;
	         var tooltipColumnNum = dataColumnNum+1;
                 columnArray.push(dataColumnNum);
	         columnArray.push(tooltipColumnNum);
	     }
         }
         <!-- document.write(columnArray); -->

         var view = new google.visualization.DataView(dataTable);
         view.setColumns(columnArray);
         chart.draw(view,options);
      }
   """
    print options % (det_label, fig_label, fig_label, form_num)

def detector_o_form(channel_label, num_channels, channel_title, fig_label):
    print "<form>"
    for i in range(1,num_channels+1):
        if not channel_title:
            print "<input name=\"channel\" type=\"checkbox\" value=\"%d\">%s %02d" % (i,channel_label,i)
        else:
            print "<input name=\"channel\" type=\"checkbox\" value=\"%d\">%s" % (i,channel_title[i-1])
        if i % 10 == 0:
            print "<br>"
    print ("<input type=\"button\" id=\"%s_button\" onclick=\"init()\" value=\"Display\">" % (fig_label))
    print "</form>"
