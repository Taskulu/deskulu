(function ($) {

Drupal.behaviors.mandrill_reports = {
  attach: function (context, settings) {
    google.load("visualization", "1", {packages:["corechart"], "callback":drawCharts});

    function drawCharts() {
      var dataTableVol = new google.visualization.DataTable();
      dataTableVol.addColumn('datetime', Drupal.t('Date'));
      dataTableVol.addColumn('number', Drupal.t('Delivered'));
      dataTableVol.addColumn('number', Drupal.t('Bounced'));
      dataTableVol.addColumn('number', Drupal.t('Rejected'));

      for (var key in settings.mandrill_reports.volume) {
        dataTableVol.addRow([
          new Date(settings.mandrill_reports.volume[key]['date']),
          settings.mandrill_reports.volume[key]['sent'],
          settings.mandrill_reports.volume[key]['bounced'],
          settings.mandrill_reports.volume[key]['rejected']
        ]);
      }

      var options = {
        pointSize: 5,
        hAxis: {format: 'MM/dd/y hh:mm aaa'}
      };

      var chart = new google.visualization.LineChart(document.getElementById('mandrill-volume-chart'));
      chart.draw(dataTableVol, options);

      var dataTableEng = new google.visualization.DataTable();
      dataTableEng.addColumn('datetime', Drupal.t('Date'));
      dataTableEng.addColumn('number', Drupal.t('Open rate'));
      dataTableEng.addColumn('number', Drupal.t('Click rate'));

      for (var key in settings.mandrill_reports.engagement) {
        dataTableEng.addRow([
          new Date(settings.mandrill_reports.engagement[key]['date']),
          settings.mandrill_reports.engagement[key]['open_rate'],
          settings.mandrill_reports.engagement[key]['click_rate']
        ]);
      }

      var chart = new google.visualization.LineChart(document.getElementById('mandrill-engage-chart'));
      chart.draw(dataTableEng, options);
    }
  }
}

})(jQuery);
