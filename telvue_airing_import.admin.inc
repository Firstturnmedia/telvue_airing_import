<?php

/**
 * Callback for module settings page.
 */
function telvue_airing_import_settings_form($form, &$form_state) {
  $form = array();

  $form['telvue_airing_import']['telvue_airing_import_xml_feed_url_1'] = array(
    '#type' => 'textfield',
    '#title' => t('XML Feed/Source URL 1'),
    '#default_value' => variable_get('telvue_airing_import_xml_feed_url_1', ''),
    '#description' => 'Set the base url from which to retrieve the airings xml feed.',
    '#size' => 120,
  );

  $form['telvue_airing_import']['telvue_airing_import_xml_feed_url_2'] = array(
    '#type' => 'textfield',
    '#title' => t('XML Feed/Source URL 2'),
    '#default_value' => variable_get('telvue_airing_import_xml_feed_url_2', ''),
    '#description' => 'Set the base url from which to retrieve the airings xml feed.',
    '#size' => 120,
  );

  $form['telvue_airing_import']['telvue_airing_import_xml_feed_url_3'] = array(
    '#type' => 'textfield',
    '#title' => t('XML Feed/Source URL 3'),
    '#default_value' => variable_get('telvue_airing_import_xml_feed_url_3', ''),
    '#description' => 'Set the base url from which to retrieve the airings xml feed.',
    '#size' => 120,
  );

  $form['telvue_airing_import']['telvue_airing_import_xml_feed_url_4'] = array(
    '#type' => 'textfield',
    '#title' => t('XML Feed/Source URL 4'),
    '#default_value' => variable_get('telvue_airing_import_xml_feed_url_4', ''),
    '#description' => 'Set the base url from which to retrieve the airings xml feed.',
    '#size' => 120,
  );

  $form['telvue_airing_import']['telvue_airing_update_future'] = array(
    '#type' => 'checkbox',
    '#title' => t('Update Airings in the Future.'),
    '#default_value' => variable_get('telvue_airing_update_future', ''),
    '#description' => 'Check this box to update all airings in the future.',
    '#size' => 120,
  );

  // Exclude if no show
  $form['telvue_airing_import']['telvue_airing_no_show_exclude'] = array(
    '#type' => 'checkbox',
    '#title' => t('If No "Drupal" Show, exclude Airing from import'),
    '#default_value' => variable_get('telvue_airing_no_show_exclude', ''),
    '#description' => 'Check this box to exclude any airing that does not have a matching show node in Drupal.',
    '#size' => 120,
  );

  // Exclude if no series
  $form['telvue_airing_import']['telvue_airing_no_series_exclude'] = array(
    '#type' => 'checkbox',
    '#title' => t('If No "Drupal" Series, exclude Airing from import'),
    '#default_value' => variable_get('telvue_airing_no_series_exclude', ''),
    '#description' => 'Check this box to exclude any airing that does not have a matching series node in Drupal.',
    '#size' => 120,
  );

  return system_settings_form($form);
}

/**
 * Delete all Form
 */
function telvue_airing_import_delete_all_form($form, &$form_state) {
  $form['telvue_airing_import_message'] = array (
    '#type' => 'item',
    '#markup' => 'Clicking this button will delete **ALL** Airing entities and related field data.',
  );

  $form['telvue_airing_import_delete_all'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );

  return $form;
}

/**
 * Delete all Form Submit
 */
function telvue_airing_import_delete_all_form_submit($form, &$form_state) {
  // Get count of shows for max count
  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'airing');

  // Get max count
  $max_count = $query->count()->execute();

  if ($max_count > 0) {
    $form_values = array(
  	  'max' => $max_count,
    );

    $operations[] = array('telvue_airing_import_delete_all_batch_op', array($form_values));

    $batch = array(
      'operations' => $operations,
      'finished' => 'telvue_airing_import_delete_all_batch_finished',
      'title' => t("Delete All Airings Batch"),
      'init_message' => t('Deleteing Airings'),
      'error_message' => t('Error!'),
      // This is needed b/c our batch process is not in the .module file.
      'file' => drupal_get_path('module', 'telvue_airing_import') . '/telvue_airing_import.admin.inc'
    );

    batch_set($batch);
  }
  else {
    drupal_set_message(t('There are airings to delete.'));
  }
}

/**
 * Batch op to delete all airings.
 */
function telvue_airing_import_delete_all_batch_op($form_values, &$context) {
  if (empty($context['sandbox'])) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['current_airing'] = 0;
    $context['sandbox']['max'] = $form_values['max'];
  }
  // Number of airings to process in each batch op
  $limit = 50;

  if ($context['sandbox']['progress'] != $context['sandbox']['max']) {
    $query = new EntityFieldQuery();
      $query->entityCondition('entity_type', 'airing');
      $query->propertyCondition('airing_id', $context['sandbox']['current_airing'], '>');
      $query->range(0, $limit);
    $result = $query->execute();

    if (isset($result['airing'])) {
      $airing_ids = array_keys($result['airing']);
      // Load airings
      $airings = cm_airing_load_multiple($airing_ids);

      // Iterate over airings and delete.
      foreach ($airings as $airing) {
        // Delete airing
        cm_airing_delete($airing);

        // Update context vars
        $context['results'][] = $airing->airing_id . ' : ' . check_plain($airing->airing_id);
        $context['sandbox']['progress']++;
        $context['sandbox']['current_airing'] = $airing->airing_id;
        $context['message'] = 'Processed ' . $context['sandbox']['progress'] . ' of ' . $context['sandbox']['max'] . ' airings.';
      }

    }
  }
  else {
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['max'];
  }

  if ($context['sandbox']['progress'] != $context['sandbox']['max']) {
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['max'];
  }
}

/**
 * Batch finished delete all airings.
 */
function telvue_airing_import_delete_all_batch_finished($success, $results, $operations) {
  if ($success) {
    drupal_set_message(t('All airings and related field data deleted! Really hope you intended to do this. :/'));
  }
  else {
    // An error occurred.
    // $operations contains the operations that remained unprocessed.
    $error_operation = reset($operations);
    drupal_set_message(
      t('An error occurred while processing @operation with arguments : @args',
        array(
          '@operation' => $error_operation[0],
          '@args' => print_r($error_operation[0], TRUE),
        )
      )
    );
  }
}

/**
 * Import all Form
 */
function telvue_airing_import_import_all_form($form, &$form_state) {
  /*$form['telvue_airing_import_all_message'] = array(
    '#type' => 'item',
    '#markup' => 'Clicking this button will import all Airings.',
  );
  */

  // Get feed urls from vars table
  $feed_1 = variable_get('telvue_airing_import_xml_feed_url_1', '');
  $feed_2 = variable_get('telvue_airing_import_xml_feed_url_2', '');
  $feed_3 = variable_get('telvue_airing_import_xml_feed_url_3', '');
  $feed_4 = variable_get('telvue_airing_import_xml_feed_url_4', '');

  $feed_info_markup = '<h2>Source Feeds Key</h2>';

  if (!empty($feed_1)) {
    $feed_info_markup .= '<label>Telvue Airing Import Feed 1:</label> <a target="_blank" href="' . $feed_1 . '">' . $feed_1 . '</a>';
  }
  if (!empty($feed_2)) {
    $feed_info_markup .= '<label>Telvue Airing Import Feed 2:</label> <a target="_blank" href="' . $feed_2 . '">' . $feed_2 . '</a>';
  }
  if (!empty($feed_3)) {
    $feed_info_markup .= '<label>Telvue Airing Import Feed 3:</label> <a target="_blank" href="' . $feed_3 . '">' . $feed_3 . '</a>';
  }
  if (!empty($feed_4)) {
    $feed_info_markup .= '<label>Telvue Airing Import Feed 4:</label> <a target="_blank" href="' . $feed_4 . '">' . $feed_4 . '</a>';
  }

  $form['telvue_airing_import_all_info'] = array(
    '#type' => 'item',
    '#markup' => $feed_info_markup,
  );

  // Select options
  // Get all telvue_airing_import feed sources from elysia_cron table.
  $cron_tasks = db_select('elysia_cron', 'ec')
    ->fields('ec', array('name'))
    ->condition('name', '%' . db_like('telvue_airing_import') . '%', 'LIKE')
    ->execute()
    ->fetchAll();

  // For each job that matches, run cron.
  // @todo this will probably time out if there are a lot of feeds. What to do?
  if (isset($cron_tasks)) {
    foreach ($cron_tasks as $cron_task) {
      $feeds_options[$cron_task->name] = $cron_task->name;
    }
  }

  // Cron Task Select
  $form['telvue_airing_import_all_select'] = [
    '#type' => 'select',
    '#title' => t('Available Telvue Feeds'),
    '#multiple' => TRUE,
    '#description' => 'Select the Telvue Feeds to Manually Import. Multiple Feeds can be selected.',
    '#required' => TRUE,
    '#options' => $feeds_options,
  ];

  $form['telvue_airing_import_all'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );

  return $form;
}

/**
 * Import all Form Submit
 */
function telvue_airing_import_import_all_form_submit($form, &$form_state) {
  $form_values = $form_state['values']['telvue_airing_import_all_select'];

  foreach ($form_values as $cron_job) {
    $cron = elysia_cron_run_job($cron_job, $ignore_time = TRUE, $ignore_disable = TRUE);
  }
}
