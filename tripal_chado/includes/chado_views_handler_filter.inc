<?php
/**
 * @file
 * Contains views filter handlers that provide chado support.
 */

/**
 * Adds support for chado foreign key filters.
 */
class chado_views_handler_filter_fk extends views_handler_filter {

  /**
   * {@inheritdoc}
   */
  function query() {

    // Adds joins to chado_entity and the chado table this field is from.
    $alias = _chado_views_add_table_joins($this);

    // Now add the restriction to the chado table as specified by user input.
    if (sizeof($this->value) == 1) {
      $value = array_pop($this->value);
      $field = $alias .'.'. $this->definition['chado_field'];
      $this->query->add_where($this->options['group'], $field, $value, '=');
    }
    elseif (sizeof($this->value) > 1) {
      $field = $alias .'.'. $this->definition['chado_field'];
      $this->query->add_where($this->options['group'], $field, $this->value, 'IN');
    }
    // @todo handle multiple values.
     
  }

  /**
   * {@inheritdoc}
   */
  function value_form(&$form, &$form_state) {
    parent::value_form($form, $form_state);

    $this->options['values_form_type'] = (isset($this->options['values_form_type'])) ? $this->options['values_form_type'] : 'textfield';

    if (preg_match('/select/', $this->options['values_form_type'])) {

      //Select List
      $form['value'] = array(
        '#type' => 'select',
        '#title' => t('%label', array('%label' => $this->options['expose']['label'])),
        '#options' => $this->get_select_options(),
        '#default_value' => $this->value,
      );

      //if ($this->options['select_multiple']) {
        //$form['value']['#multiple'] = TRUE;
      //}
    }
    else {

      $form['value'] = array(
        '#type' => 'textfield',
        '#title' => t('%label', array('%label' => $this->options['expose']['label'])),
        '#default_value' => $this->value,
      );

    }
  }

  /**
   * {@inheritdoc}
   */
  function exposed_form(&$form, &$form_state) {
    parent::exposed_form($form, $form_state);

    if (isset($this->options['select_multiple'])) {
      if ($this->options['select_multiple']) {

        if (isset($this->options['expose']['identifier'])) {
          $id = $this->options['expose']['identifier'];
        }
        else {
          $id = $this->options['id'];
        }
        $form[$id]['#multiple'] = TRUE;
      }
    }
  }
  
  function get_select_options() {

    $name_field = 'common_name';
        
    // Using a "Loose Index Scan" to get a list of all the unique values for
    // the name in the table referenced by the foreign key constraint.
    // See https://wiki.postgresql.org/wiki/Loose_indexscan
    $sql = "WITH RECURSIVE t AS (
          SELECT MIN(filter_table.!id_field) AS col
            FROM {!filter_table} filter_table
            LEFT JOIN {!foreign_table} foreign_table ON filter_table.!id_field=foreign_table.!id_field
          UNION ALL
          SELECT (
              SELECT MIN(filter_table.!id_field)
              FROM {!filter_table} filter_table
              LEFT JOIN {!foreign_table} foreign_table ON filter_table.!id_field=foreign_table.!id_field
              WHERE filter_table.!id_field > col
            )
            FROM t WHERE col IS NOT NULL
        )
        SELECT !id_field as id, !name_field as name
          FROM {!foreign_table}
          WHERE !id_field IN (SELECT col FROM t where col IS NOT NULL)
          ORDER BY !name_field ASC";
    $sql = format_string($sql, array(
      '!filter_table' => $this->definition['chado_table'],
      '!foreign_table' => $this->definition['foreign_key']['right_table'],
      '!id_field' => $this->definition['chado_field'],
      '!name_field' => $name_field
    ));

    $resource = chado_query($sql);
    $options = array();

    if ($this->options['select_optional']) {
      $options['All'] = '- Any -';
    }

    foreach ($resource as $r) {
      $options[$r->id] = $r->name;
    }

    return $options;
  }

  /**
   * {@inheritdoc}
   */
  function has_extra_options() {
    return TRUE;
  }

  /**
   * {@inheritdoc}
   */
  function extra_options_form(&$form, &$form_state) {
    parent::extra_options_form($form, $form_state);

    $form['values_form_type'] = array(
      '#type' => 'radios',
      '#title' => t('Filter Type'),
      '#options' => array(
        'textfield' => 'Text Field',
        'select' => 'Drop-Down Box',
      ),
      '#default_value' => $this->options['values_form_type'],
    );

    // @todo: implement.
    //$form['show_all'] = array(
      //'#type' => 'checkbox',
      //'#title' => t('Show All'),
      //'#description' => t('When selected all terms from the controlled vocaulbary used by the table will be shown where the default is to only show those that are used.'),
      //'#default_value' => $this->options['show_all'],
    //);
 
    // @todo: implement.
    //$form['select_multiple'] = array(
      //'#type' => 'checkbox',
      //'#title' => t('Select Multiple'),
      //'#description' => t('Allows more then one option to be selected.'),
      //'#default_value' => $this->options['select_multiple'],
    //);

    $form['select_optional'] = array(
      '#type' => 'checkbox',
      '#title' => t('Optional'),
      '#description' => t('Adds --Any-- to the available options.'),
      '#default_value' => $this->options['select_optional'],
    );

    $form['max_length'] = array(
      '#type' => 'textfield',
      '#title' => t('Max Width'),
      '#description' => t('Specify the maximum width of the select box'),
      '#default_value' => $this->options['max_length'],
    );

    $form['note'] = array(
      '#type' => 'markup',
      '#value' => t('<strong><font color="red">Note:</font></strong> If another filter exists for the same table then ' . 'the values shown in the drop box will only include those from rows that are not filtered.'),
    );

    return $form;

  }

  /**
   * {@inheritdoc}
   */
  function extra_options_submit($form, &$form_state) {
    $this->options['values_form_type'] = $form_state['values']['options']['values_form_type'];
    $this->options['select_multiple'] = $form_state['values']['options']['select_multiple'];
    $this->options['select_optional'] = $form_state['values']['options']['select_optional'];
    $this->options['max_length'] = $form_state['values']['options']['max_length'];
    $this->options['show_all'] = $form_state['values']['options']['show_all'];
  }

  /**
   * {@inheritdoc}
   */
  function extra_options_options() {
    $this->options['values_form_type'] = 'textfield';
    $this->options['select_multiple'] = FALSE;
    $this->options['select_optional'] = FALSE;
    $this->options['max_length'] = 40;
    $this->options['show_all'] = FALSE;
  }

  /**
   * {@inheritdoc}
   */
  function option_definition() {
    $options = parent::option_definition();

    $options['values_form_type'] = array(
      'default' => 'textfield',
      'export' => TRUE,
    );
    $options['select_multiple'] = array(
      'default' => FALSE,
      'bool' => TRUE,
      'export' => TRUE,
    );
    $options['select_optional'] = array(
      'default' => FALSE,
      'bool' => TRUE,
      'export' => TRUE,
    );
    $options['show_all'] = array(
      'default' => FALSE,
      'bool' => TRUE,
      'export' => TRUE,
    );
    $options['max_length'] = array(
      'default' => 40,
      'export' => TRUE,
    );

    return $options;
  }
}

/**
 * Adds chado support to the string filter.
 */
class chado_views_handler_filter_string extends views_handler_filter_string {

  function query() {
    
    // Adds joins to chado_entity and the chado table this field is from.
    $alias = _chado_views_add_table_joins($this);

    // Finally add the restriction on the chado table including the value entered by the filter.
    // Notice that we don't call $query->add_where directly. This is so that the options
    // specified for the comparison operator are used (ie: contains).
    $info = $this->operators();
    $field = $alias .'.'. $this->definition['chado_field'];
    if (!empty($info[$this->operator]['method'])) {
      $this->{$info[$this->operator]['method']}($field);
    }
  }
}

/**
 * A helper function for adding joins in the views_handler::query() method
 * to connect a TripalEntity with it's chado tables.
 *
 * @param object $views_handler
 *   $this from within the views_handler::query() method.
 */
function _chado_views_add_table_joins(&$handler) {

  // First we need to join to the chado_entity table where the link between an 
  // entity and it's chado record is stored.
  $join = new views_join();
  $join->construct('chado_entity', $handler->table, 'id', 'chado_entity_id');
  $alias = $handler->query->add_relationship('chado_entity', $join, $handler->table_alias, $handler->relationship);

  // Restrict the chado_entity join to only return the table this field is from.
  $handler->query->add_where(0, $alias .'.data_table', $handler->definition['chado_table'], '=');
  
  // Now, we need to join from chado_entity to the chado table needed by this field.
  // This only works if the field is from the base table.
  // @todo: fix handler to work with non-base tables.
  $join = new views_join();
  $chado_table = 'chado.'.$handler->definition['chado_table'];
  $join->construct($chado_table, 'chado_entity', 'record_id', $handler->definition['chado_table'].'_id');
  $alias = $handler->query->add_relationship('chado_'.$handler->definition['chado_table'], $join, $handler->table_alias, $handler->relationship);

  return $alias;
}